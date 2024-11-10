#ifndef WEAPON_PHYSGUN_H
#define WEAPON_PHYSGUN_H

#include "cbase.h"
#include "weapon_physbase.h"
#include "beam_shared.h"  // For CBeam
#include "props.h"         // For CPhysicsObject

class CWeaponPhysgun : public CWeaponPhysBase
{
    DECLARE_CLASS(CWeaponPhysgun, CWeaponPhysBase);
public:
    CWeaponPhysgun();

    void Precache(void);
    void PrimaryAttack(void);
    void SecondaryAttack(void);
    void ItemPostFrame(void);
    void CreateLaserBeam(void);
    void UpdateLaserBeam(void);
    void PickupObject(CBaseEntity *pEntity);
    void ReleaseObject();
    void FreezeObject(bool bFreeze);
    void UnfreezeAllObjects();  // Unfreezes all recently frozen objects

private:
    CBeam *m_pLaserBeam;            // The laser beam entity
    bool m_bLaserActive;            // Is the laser currently active?
    CBaseEntity *m_hHeldObject;     // The object being held
    bool m_bObjectFrozen;           // Is the held object frozen?
    bool m_bHoldingObject;          // Are we currently holding an object?
    
    // List of frozen objects (recently frozen)
    CUtlVector<CPhysicsObject*> m_vecFrozenObjects;
};

#endif // WEAPON_PHYSGUN_H
