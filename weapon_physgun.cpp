#include "cbase.h"
#include "weapon_physgun.h"
#include "in_buttons.h"
#include "beam_shared.h"
#include "props.h"
#include "eventqueue.h"
#include "util.h"

LINK_ENTITY_TO_CLASS(weapon_physgun, CWeaponPhysgun);

CWeaponPhysgun::CWeaponPhysgun()
{
    m_pLaserBeam = NULL;
    m_hHeldObject = NULL;
    m_bLaserActive = false;
    m_bObjectFrozen = false;
    m_bHoldingObject = false;
}

void CWeaponPhysgun::Precache()
{
    PrecacheModel("models/weapons/v_physcannon.mdl");
    PrecacheParticleSystem("physgun_beam");  // Preload the laser particle system
}

void CWeaponPhysgun::PrimaryAttack()
{
    if (m_bHoldingObject)
    {
        // Release the held object if we're currently holding one
        ReleaseObject();
    }
    else
    {
        // Try to pick up an object
        trace_t tr;
        Vector vecStart = GetOwner()->EyePosition();
        Vector vecEnd = vecStart + GetOwner()->EyeVectors()[0] * 1000;  // Max reach

        UTIL_TraceLine(vecStart, vecEnd, MASK_SOLID, GetOwner(), COLLISION_GROUP_NONE, &tr);

        // If we hit a physics object, try to pick it up
        if (tr.m_pEnt && tr.m_pEnt->IsPhysicsObject())
        {
            PickupObject(tr.m_pEnt);
        }
    }
}

void CWeaponPhysgun::SecondaryAttack()
{
    if (m_bHoldingObject)
    {
        // Toggle the freeze state of the held object
        FreezeObject(!m_bObjectFrozen);
    }
}

void CWeaponPhysgun::ItemPostFrame()
{
    BaseClass::ItemPostFrame();

    // If laser isn't active, activate it
    if (!m_bLaserActive)
    {
        CreateLaserBeam();
    }

    // Update the laser beam's position
    UpdateLaserBeam();

    // If the freeze/unfreeze toggle is enabled, check for 'R' key press
    if (GetOwner()->m_nButtons & IN_RELOAD)  // IN_RELOAD corresponds to the R key
    {
        UnfreezeAllObjects();  // Call to unfreeze all frozen objects when R is pressed
    }
}

void CWeaponPhysgun::CreateLaserBeam()
{
    if (!m_pLaserBeam)
    {
        m_pLaserBeam = CBeam::BeamCreate("sprites/laser.vmt", 1.0f); // You can use a custom laser sprite
        if (m_pLaserBeam)
        {
            m_pLaserBeam->SetFlags(BEAM_FLAG_TRAIL);  // Optional: Adds a trail effect
            m_pLaserBeam->SetWidth(0.1f);              // Laser beam width
            m_pLaserBeam->SetEndWidth(0.1f);           // End width
            m_pLaserBeam->SetLife(0.1f);               // Lifetime
            m_pLaserBeam->SetColor(255, 255, 255);     // Laser color (white)
        }
        m_bLaserActive = true; // Mark laser as active
    }
}

void CWeaponPhysgun::UpdateLaserBeam()
{
    if (!m_pLaserBeam) return;

    trace_t tr;
    Vector vecEnd = GetAbsOrigin() + GetOwner()->EyeVectors()[0] * 1000; // Laser range
    
    UTIL_TraceLine(GetOwner()->EyePosition(), vecEnd, MASK_SOLID, GetOwner(), COLLISION_GROUP_NONE, &tr);

    m_pLaserBeam->SetStartPos(GetOwner()->EyePosition());
    m_pLaserBeam->SetEndPos(tr.endpos);

    DispatchParticleEffect("physgun_beam", tr.endpos, QAngle(0, 0, 0)); // Particle effect at the end
}

void CWeaponPhysgun::PickupObject(CBaseEntity *pEntity)
{
    // Check if we can pick up the object (ensure it's a valid physics object)
    if (pEntity && pEntity->IsPhysicsObject())
    {
        CPhysicsObject *pPhysicsObject = pEntity->VPhysicsGetObject();
        if (pPhysicsObject)
        {
            // Create a constraint to hold the object in place
            IPhysicsConstraintGroup *pConstraintGroup = pPhysicsObject->GetConstraintGroup();
            pConstraintGroup->SetConstraintFlags(CF_DONT_ALLOW_ROTATION);
            pConstraintGroup->EnableMotion(false);

            m_hHeldObject = pEntity;
            m_bHoldingObject = true;
        }
    }
}

void CWeaponPhysgun::ReleaseObject()
{
    if (m_hHeldObject)
    {
        CPhysicsObject *pPhysicsObject = m_hHeldObject->VPhysicsGetObject();
        if (pPhysicsObject)
        {
            // Release the constraint, allowing the object to move again
            IPhysicsConstraintGroup *pConstraintGroup = pPhysicsObject->GetConstraintGroup();
            pConstraintGroup->EnableMotion(true);
        }

        m_hHeldObject = NULL;
        m_bHoldingObject = false;
    }
}

void CWeaponPhysgun::FreezeObject(bool bFreeze)
{
    if (m_hHeldObject)
    {
        CPhysicsObject *pPhysicsObject = m_hHeldObject->VPhysicsGetObject();
        if (pPhysicsObject)
        {
            if (bFreeze)
            {
                // Freeze the object
                pPhysicsObject->EnableMotion(false);
                m_vecFrozenObjects.AddToTail(pPhysicsObject);  // Add to list of recently frozen objects
                m_bObjectFrozen = true;
            }
            else
            {
                // Unfreeze the object
                pPhysicsObject->EnableMotion(true);
                m_bObjectFrozen = false;
            }
        }
    }
}

// This method unfreezes all recently frozen objects
void CWeaponPhysgun::UnfreezeAllObjects()
{
    for (int i = 0; i < m_vecFrozenObjects.Count(); ++i)
    {
        if (m_vecFrozenObjects[i])
        {
            m_vecFrozenObjects[i]->EnableMotion(true);  // Unfreeze the object
        }
    }
    m_vecFrozenObjects.RemoveAll();  // Clear the list after unfreezing
}
