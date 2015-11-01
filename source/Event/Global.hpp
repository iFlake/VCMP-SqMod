#ifndef _SQMOD_EVENT_GLOBAL_HPP_
#define _SQMOD_EVENT_GLOBAL_HPP_

// ------------------------------------------------------------------------------------------------
#include "Common.hpp"
#include "Shared.hpp"

// ------------------------------------------------------------------------------------------------
#include "Entity/Blip.hpp"
#include "Entity/Checkpoint.hpp"
#include "Entity/Keybind.hpp"
#include "Entity/Object.hpp"
#include "Entity/Pickup.hpp"
#include "Entity/Player.hpp"
#include "Entity/Sphere.hpp"
#include "Entity/Sprite.hpp"
#include "Entity/Textdraw.hpp"
#include "Entity/Vehicle.hpp"

// ------------------------------------------------------------------------------------------------
#include <chrono>

// ------------------------------------------------------------------------------------------------
namespace SqMod {

// ------------------------------------------------------------------------------------------------
/** Internal class used to reduce code duplication when filtering entities in a global event.
 *
*/
template < class T > class GlobalFilter
{
protected:

    // --------------------------------------------------------------------------------------------
    friend class GlobalEvent; /* The parent class must be able to directly access our stuff */

    // --------------------------------------------------------------------------------------------
    typedef typename Ent< T >::Set Container; /* Size efficient container used as the filter */

public:

    // --------------------------------------------------------------------------------------------
    typedef T               Type; /* The type of entity that this filter applies to */

    // --------------------------------------------------------------------------------------------
    typedef Reference< T >  RefType; /* Short name for the entity reference type */
    typedef Ent< T >        EntType; /* Short name for the entity specification structure */

private:

    /* --------------------------------------------------------------------------------------------
     * Construct the filter for the specified event.
    */
    GlobalFilter(GlobalEvent * evt) noexcept
        : m_Filter(), m_Event(evt)
    {
        /* The filter is empty so there's nothing to hook to at this moment! */
    }

    /* --------------------------------------------------------------------------------------------
     * Copy constructor (disabled)
    */
    GlobalFilter(const GlobalFilter< T > &) = delete;

    /* --------------------------------------------------------------------------------------------
     * Move constructor (disabled)
    */
    GlobalFilter(GlobalFilter< T > &&) = delete;

public:

    /* --------------------------------------------------------------------------------------------
     * Destructor
    */
    ~GlobalFilter()
    {
        Unhook(); /* No need to listen to the destroy signal of filtered entities anymore! */
    }

    /* --------------------------------------------------------------------------------------------
     * Copy assignment operator
    */
    GlobalFilter< T > & operator = (const GlobalFilter< T > & o) noexcept;

    /* --------------------------------------------------------------------------------------------
     * Move assignment operator (disabled)
    */
    GlobalFilter< T > & operator = (GlobalFilter< T > &&) = delete;

    /* --------------------------------------------------------------------------------------------
     * Convert this class to an integer with the count of the filtered entities
    */
    operator SQInt32 () const noexcept
    {
        return _SCI32(m_Filter.count());
    }

    /* --------------------------------------------------------------------------------------------
     * Used by the script to compare two instances of this type.
    */
    SQInt32 Cmp(const GlobalFilter< T > & o) const noexcept
    {
        // Rule out equality first
        if (m_Filter.count() == o.m_Filter.count())
        {
            return 0;
        }
        // Rule out by size second
        else if (m_Filter.count() > o.m_Filter.count())
        {
            return 1;
        }
        else
        {
            return -1;
        }
    }

    /* --------------------------------------------------------------------------------------------
     * Used by the script to convert this type to a string.
    */
    String ToString() const noexcept
    {
        return NToS(m_Filter.count());
    }

    /* --------------------------------------------------------------------------------------------
     * Include the specified entity in the filter.
    */
    bool Include(const RefType & ent, SQInt32 header) noexcept;

    /* --------------------------------------------------------------------------------------------
     * Include the specified entity in the filter.
    */
    bool Include(const RefType & ent) noexcept
    {
        return Include(ent, 0);
    }

    /* --------------------------------------------------------------------------------------------
     * Exclude the specified entity from the filter.
    */
    bool Exclude(const RefType & ent, SQInt32 header) noexcept;

    /* --------------------------------------------------------------------------------------------
     * Exclude the specified entity from the filter.
    */
    bool Exclude(const RefType & ent) noexcept
    {
        return Exclude(ent, 0);
    }

    /* --------------------------------------------------------------------------------------------
     * Test whether the specified entity is included in the filter.
    */
    bool Enabled(const RefType & ent) const noexcept
    {
        if (ent)
        {
            return m_Filter[_SCU32(ent)];
        }
        else
        {
            LogWrn("Cannot test whether a <%s> entity is filtered or not using an invalid instance: %d", \
                    EntType::Name, _SCI32(ent));
        }

        return false;
    }

    /* --------------------------------------------------------------------------------------------
     * Count all the entities included in the filter.
    */
    SQInt32 Count() const noexcept
    {
        return _SCI32(m_Filter.count());
    }

    /* --------------------------------------------------------------------------------------------
     * Remove all the entities included in the filter.
    */
    void Clear(SQInt32 header) noexcept;

    /* --------------------------------------------------------------------------------------------
     * Reverse the filter to exclude currently include entities.
    */
    void Flip(SQInt32 header) noexcept;

    /* --------------------------------------------------------------------------------------------
     * Test whether we have any entity included in this filter?
    */
    bool Any() const noexcept
    {
        return m_Filter.any();
    }

    /* --------------------------------------------------------------------------------------------
     * Test whether we have no entity included in this filter?
    */
    bool None() const noexcept
    {
        return m_Filter.none();
    }

    /* --------------------------------------------------------------------------------------------
     * Test whether we have all entities included in this filter?
    */
    bool All() const noexcept
    {
        return m_Filter.all();
    }

    /* --------------------------------------------------------------------------------------------
     * Exclude the entity from filter when the parent event isn't attached to the destroy signal.
    */
    void Destroyed(SQInt32 id, SQInt32 /* header */, Object & /* payload */) noexcept
    {
        Release(id);
    }

protected:

    /* --------------------------------------------------------------------------------------------
     * Used by either the parent event or it self to release an entity from the filter.
    */
    void Release(SQInt32 id) noexcept;

    /* --------------------------------------------------------------------------------------------
     * Hook to the entity destroy signal that the parent didn't.
    */
    void Hook() noexcept;

    /* --------------------------------------------------------------------------------------------
     * Unhook from the entity destroy signal that the parent didn't.
    */
    void Unhook() noexcept;

private:

    // --------------------------------------------------------------------------------------------
    Container       m_Filter; /* Entity filter */

    // --------------------------------------------------------------------------------------------
    GlobalEvent*    m_Event; /* Parent event */
};

/* ------------------------------------------------------------------------------------------------
 * ...
*/
class GlobalEvent
{
protected:

    // --------------------------------------------------------------------------------------------
    template < class T > friend class GlobalFilter;

    // --------------------------------------------------------------------------------------------
    typedef GlobalFilter< CBlip >           BlipFilter;
    typedef GlobalFilter< CCheckpoint >     CheckpointFilter;
    typedef GlobalFilter< CKeybind >        KeybindFilter;
    typedef GlobalFilter< CObject >         ObjectFilter;
    typedef GlobalFilter< CPickup >         PickupFilter;
    typedef GlobalFilter< CPlayer >         PlayerFilter;
    typedef GlobalFilter< CSphere >         SphereFilter;
    typedef GlobalFilter< CSprite >         SpriteFilter;
    typedef GlobalFilter< CTextdraw >       TextdrawFilter;
    typedef GlobalFilter< CVehicle >        VehicleFilter;

    // --------------------------------------------------------------------------------------------
    typedef std::chrono::time_point< std::chrono::steady_clock > TimePoint;

public:

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    GlobalEvent() noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    GlobalEvent(SQInt32 type) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    GlobalEvent(SQInt32 type, bool suspended) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    GlobalEvent(const GlobalEvent &) = delete;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    GlobalEvent(GlobalEvent &&) = delete;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    ~GlobalEvent();

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    GlobalEvent & operator = (const GlobalEvent &) = delete;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    GlobalEvent & operator = (GlobalEvent &&) = delete;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    bool operator == (const GlobalEvent & o) const noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    bool operator != (const GlobalEvent & o) const noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    bool operator < (const GlobalEvent & o) const noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    bool operator > (const GlobalEvent & o) const noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    bool operator <= (const GlobalEvent & o) const noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    bool operator >= (const GlobalEvent & o) const noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    operator SQInt32 () const noexcept
    {
        return m_Type;
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    operator bool () const noexcept
    {
        return (m_Type != EVT_UNKNOWN && m_Type < EVT_COUNT);
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    operator ! () const noexcept
    {
        return (m_Type == EVT_UNKNOWN || m_Type >= EVT_COUNT);
    }

    /* --------------------------------------------------------------------------------------------
      * ...
     */
    void VMClose() noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    SQInt32 Cmp(const GlobalEvent & o) const noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    const SQChar * GetName() const noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    const SQChar * GetTag() const noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void SetTag(const SQChar * tag) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    SqObj & GetData() noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void SetData(SqObj & data) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    SQInt32 GetType() const noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void SetType(SQInt32 type) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    SQInteger GetIdle() const noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void SetIdle(SQInteger millis) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    bool IsIdle() const noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    SQInt32 GetStride() const noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void SetStride(SQInt32 stride) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    SQInt32 GetIgnore() const noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void SetIgnore(SQInt32 ignore) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    SQInt32 GetPrimary() const noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void SetPrimary(SQInt32 subset) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    SQInt32 GetSecondary() const noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void SetSecondary(SQInt32 subset) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    bool GetConfined() const noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void SetConfined(bool toggle) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    bool GetSuspended() const noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void SetSuspended(bool toggle) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Function GetOnTrigger() const noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void SetOnTrigger(Function & func) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void SetOnTrigger_Env(SqObj & env, Function & func) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Function GetOnInclude() const noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void SetOnInclude(Function & func) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void SetOnInclude_Env(SqObj & env, Function & func) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Function GetOnExclude() const noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void SetOnExclude(Function & func) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void SetOnExclude_Env(SqObj & env, Function & func) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Function GetOnCleared() const noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void SetOnCleared(Function & func) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void SetOnCleared_Env(SqObj & env, Function & func) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Function GetOnRelease() const noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void SetOnRelease(Function & func) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void SetOnRelease_Env(SqObj & env, Function & func) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    bool Compatible(SQInt32 type) const noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    BlipFilter & GetBlipFilter() noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    CheckpointFilter & GetCheckpointFilter() noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    KeybindFilter & GetKeybindFilter() noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    ObjectFilter & GetObjectFilter() noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    PickupFilter & GetPickupFilter() noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    PlayerFilter & GetPlayerFilter() noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    SphereFilter & GetSphereFilter() noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    SpriteFilter & GetSpriteFilter() noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    TextdrawFilter & GetTextdrawFilter() noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    VehicleFilter & GetVehicleFilter() noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void BlipDestroyed(SQInt32 blip, SQInt32 header, Object & payload) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void CheckpointDestroyed(SQInt32 checkpoint, SQInt32 header, Object & payload) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void KeybindDestroyed(SQInt32 keybind, SQInt32 header, Object & payload) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void ObjectDestroyed(SQInt32 object, SQInt32 header, Object & payload) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void PickupDestroyed(SQInt32 pickup, SQInt32 header, Object & payload) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void PlayerDestroyed(SQInt32 player, SQInt32 header, Object & payload) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void SphereDestroyed(SQInt32 sphere, SQInt32 header, Object & payload) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void SpriteDestroyed(SQInt32 sprite, SQInt32 header, Object & payload) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void TextdrawDestroyed(SQInt32 textdraw, SQInt32 header, Object & payload) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void VehicleDestroyed(SQInt32 vehicle, SQInt32 header, Object & payload) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void BlipCustom(SQInt32 blip, SQInt32 header, Object & payload) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void CheckpointCustom(SQInt32 checkpoint, SQInt32 header, Object & payload) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void KeybindCustom(SQInt32 keybind, SQInt32 header, Object & payload) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void ObjectCustom(SQInt32 object, SQInt32 header, Object & payload) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void PickupCustom(SQInt32 pickup, SQInt32 header, Object & payload) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void PlayerCustom(SQInt32 player, SQInt32 header, Object & payload) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void SphereCustom(SQInt32 sphere, SQInt32 header, Object & payload) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void SpriteCustom(SQInt32 sprite, SQInt32 header, Object & payload) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void TextdrawCustom(SQInt32 textdraw, SQInt32 header, Object & payload) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void VehicleCustom(SQInt32 vehicle, SQInt32 header, Object & payload) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void PlayerAway(SQInt32 player, bool status) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void PlayerGameKeys(SQInt32 player, SQInt32 previous, SQInt32 current) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void PlayerRename(SQInt32 player, const SQChar * previous, const SQChar * current) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void PlayerRequestClass(SQInt32 player, SQInt32 offset) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void PlayerRequestSpawn(SQInt32 player) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void PlayerSpawn(SQInt32 player) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void PlayerStartTyping(SQInt32 player) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void PlayerStopTyping(SQInt32 player) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void PlayerChat(SQInt32 player, const SQChar * message) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void PlayerCommand(SQInt32 player, const SQChar * command) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void PlayerMessage(SQInt32 player, SQInt32 receiver, const SQChar * message) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void PlayerHealth(SQInt32 player, SQFloat previous, SQFloat current) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void PlayerArmour(SQInt32 player, SQFloat previous, SQFloat current) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void PlayerWeapon(SQInt32 player, SQInt32 previous, SQInt32 current) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void PlayerMove(SQInt32 player, const Vector3 & previous, const Vector3 & current) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void PlayerWasted(SQInt32 player, SQInt32 reason) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void PlayerKilled(SQInt32 player, SQInt32 killer, SQInt32 reason, SQInt32 body_part) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void PlayerTeamKill(SQInt32 player, SQInt32 killer, SQInt32 reason, SQInt32 body_part) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void PlayerSpectate(SQInt32 player, SQInt32 target) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void PlayerCrashreport(SQInt32 player, const SQChar * report) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void PlayerBurning(SQInt32 player, bool state) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void PlayerCrouching(SQInt32 player, bool state) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void PlayerState(SQInt32 player, SQInt32 previous, SQInt32 current) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void PlayerAction(SQInt32 player, SQInt32 previous, SQInt32 current) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void StateNone(SQInt32 player, SQInt32 previous) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void StateNormal(SQInt32 player, SQInt32 previous) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void StateShooting(SQInt32 player, SQInt32 previous) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void StateDriver(SQInt32 player, SQInt32 previous) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void StatePassenger(SQInt32 player, SQInt32 previous) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void StateEnterDriver(SQInt32 player, SQInt32 previous) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void StateEnterPassenger(SQInt32 player, SQInt32 previous) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void StateExitVehicle(SQInt32 player, SQInt32 previous) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void StateUnspawned(SQInt32 player, SQInt32 previous) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void ActionNone(SQInt32 player, SQInt32 previous) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void ActionNormal(SQInt32 player, SQInt32 previous) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void ActionAiming(SQInt32 player, SQInt32 previous) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void ActionShooting(SQInt32 player, SQInt32 previous) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void ActionJumping(SQInt32 player, SQInt32 previous) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void ActionLieDown(SQInt32 player, SQInt32 previous) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void ActionGettingUp(SQInt32 player, SQInt32 previous) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void ActionJumpVehicle(SQInt32 player, SQInt32 previous) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void ActionDriving(SQInt32 player, SQInt32 previous) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void ActionDying(SQInt32 player, SQInt32 previous) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void ActionWasted(SQInt32 player, SQInt32 previous) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void ActionEmbarking(SQInt32 player, SQInt32 previous) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void ActionDisembarking(SQInt32 player, SQInt32 previous) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void VehicleRespawn(SQInt32 vehicle) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void VehicleExplode(SQInt32 vehicle) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void VehicleHealth(SQInt32 vehicle, SQFloat previous, SQFloat current) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void VehicleMove(SQInt32 vehicle, const Vector3 & previous, const Vector3 &current) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void PickupRespawn(SQInt32 pickup) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void KeybindKeyPress(SQInt32 player, SQInt32 keybind) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void KeybindKeyRelease(SQInt32 player, SQInt32 keybind) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void VehicleEmbarking(SQInt32 player, SQInt32 vehicle, SQInt32 slot) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void VehicleEmbarked(SQInt32 player, SQInt32 vehicle, SQInt32 slot) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void VehicleDisembark(SQInt32 player, SQInt32 vehicle) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void PickupClaimed(SQInt32 player, SQInt32 pickup) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void PickupCollected(SQInt32 player, SQInt32 pickup) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void ObjectShot(SQInt32 player, SQInt32 object, SQInt32 weapon) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void ObjectBump(SQInt32 player, SQInt32 object) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void CheckpointEntered(SQInt32 player, SQInt32 checkpoint) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void CheckpointExited(SQInt32 player, SQInt32 checkpoint) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void SphereEntered(SQInt32 player, SQInt32 sphere) noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void SphereExited(SQInt32 player, SQInt32 sphere) noexcept;

protected:

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    bool Trigger() noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void Attach() noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void Detach() noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void Hook() noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void Unhook() noexcept;

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void Adaptable(SQInt32 type) noexcept;

private:

    // --------------------------------------------------------------------------------------------
    SQInt32             m_Type;

    // --------------------------------------------------------------------------------------------
    SQInt32             m_Stride;
    SQInt32             m_Ignore;

    // --------------------------------------------------------------------------------------------
    SQInt32             m_Primary;
    SQInt32             m_Secondary;

    // --------------------------------------------------------------------------------------------
    TimePoint           m_Idle;

    // --------------------------------------------------------------------------------------------
    Function            m_OnTrigger;
    Function            m_OnInclude;
    Function            m_OnExclude;
    Function            m_OnCleared;
    Function            m_OnRelease;

    // --------------------------------------------------------------------------------------------
    SqTag               m_Tag;
    SqObj               m_Data;

    // --------------------------------------------------------------------------------------------
    bool                m_Confined; /* Toggles filtering on secondary entities */
    bool                m_Suspended; /* Prevents event forwarding completely */

    // --------------------------------------------------------------------------------------------
    BlipFilter          m_Blips;
    CheckpointFilter    m_Checkpoints;
    KeybindFilter       m_Keybinds;
    ObjectFilter        m_Objects;
    PickupFilter        m_Pickups;
    PlayerFilter        m_Players;
    SphereFilter        m_Spheres;
    SpriteFilter        m_Sprites;
    TextdrawFilter      m_Textdraws;
    VehicleFilter       m_Vehicles;
};

// ------------------------------------------------------------------------------------------------
template < class T > GlobalFilter< T > & GlobalFilter< T >::operator = (const GlobalFilter< T > & o) noexcept
{
    // Make sure we're not doing self assignment, work with orphan filters or incompatible events
    if (this != &o && EntType::InEvent(m_Event->m_Type))
    {
        // Unhook from the currently filtered entities
        Unhook();
        // Copy the internal filter from the other instance
        m_Filter = o.m_Filter;
        // Hook back to the newly filtered entities
        Hook();
        /* Parent is ignored intentionally as filters should not change parents! */
    }

    return *this;
}

// ------------------------------------------------------------------------------------------------
template < class T > bool GlobalFilter< T >::Include(const RefType & ent, SQInt32 header) noexcept
{
    // Make sure the entity is valid before we proceed
    if (!ent)
    {
        LogErr("Attempting to <filter %s events> using an invalid entity instance: %d", \
                EntType::Name, _SCI32(ent));
    }
    // Make sure the entity type is allowed for this event type
    else if (!EntType::InEvent(m_Event->m_Type))
    {
        LogErr("Attempting to <filter %s events> using an incompatible event type: %s", \
                EntType::Name, GetEventName(m_Event->m_Type));
    }
    // Make sure the entity is not already included in the filter
    else if (!m_Filter[_SCU32(ent)])
    {
        // Allow the inclusion of this entity by default
        bool allow = true;
        // Do we have custom inclusion filter specified?
        if (!m_Event->m_OnInclude.IsNull())
        {
            // Ask the specified inclusion filter if this entity should be allowed
            SharedPtr< bool > ret = m_Event->m_OnInclude.Evaluate< bool, RefType, SQInt32 >(ent, header);
            // See what the custom inclusion filter said or default to disallow
            allow = !ret ? false : *ret;
        }
        // Are we allowed to include this entity instance in our filter?
        if (allow)
        {
            // Make sure the someone knows when this entity will be destroyed
            if (EntType::DestroyEvID != m_Event->m_Type)
            {
                RefType::Get(_SCI32(ent)).Destroyed().template Connect< GlobalFilter< T >, &GlobalFilter< T >::Destroyed >(this);
            }
            // Enable the specified entity instance in our filter
            m_Filter.set(_SCU32(ent), true);
        }
        // Return whether this was included or not
        return allow;
    }
    // Reaching here means failure
    return false;
}

// ------------------------------------------------------------------------------------------------
template < class T > bool GlobalFilter< T >::Exclude(const RefType & ent, SQInt32 header) noexcept
{
    // Make sure the entity is valid before we proceed
    if (!ent)
    {
        LogErr("Attempting to <unfilter %s events> using an invalid entity instance: %d", \
                EntType::Name, _SCI32(ent));
    }
    // Make sure the entity type is allowed for this event type
    else if (!EntType::InEvent(m_Event->m_Type))
    {
        LogErr("Attempting to <unfilter %s events> using an incompatible event type: %s", \
                EntType::Name, GetEventName(m_Event->m_Type));
    }
    // Make sure the entity is not already excluded fom the filter
    else if (m_Filter[_SCU32(ent)])
    {
        // Allow the exclusion of this entity by default
        bool allow = true;
        // Do we have custom exclusion filter specified?
        if (!m_Event->m_OnExclude.IsNull())
        {
            // Ask the specified exclusion filter if this entity should be allowed
            SharedPtr< bool > ret = m_Event->m_OnExclude.Evaluate< bool, RefType, SQInt32 >(ent, header);
            // See what the custom exclusion filter said or default to disallow
            allow = !ret ? false : *ret;
        }
        // Are we allowed to exclude this entity instance from our filter?
        if (allow)
        {
            // We don't need to know when this entity will be destroyed anymore
            if (EntType::DestroyEvID != m_Event->m_Type)
            {
                RefType::Get(_SCI32(ent)).Destroyed().template Disconnect< GlobalFilter< T >, &GlobalFilter< T >::Destroyed >(this);
            }
            // Disable the specified entity instance in our filter
            m_Filter.set(_SCU32(ent), false);
        }
        // Return whether this was excluded or not
        return allow;
    }
    // Reaching here means failure
    return false;
}

// ------------------------------------------------------------------------------------------------
template < class T > void GlobalFilter< T >::Clear(SQInt32 header) noexcept
{
    // Make sure the filter is compatible with the specified event type
    if (!EntType::InEvent(m_Event->m_Type))
    {
        LogWrn("Attempting to <clear %s filter> using an incompatible event type: %s", \
                EntType::Name, GetEventName(m_Event->m_Type));
    }
    // Don't even attempt to clear if there's nothing to be cleared
    else if (m_Filter.any())
    {
        // Do we have to notify someone that the filter is about to be cleared?
        if (!m_Event->m_OnCleared.IsNull())
        {
            m_Event->m_OnCleared.Execute< SQInt32 >(header);
        }
        // Unhook from the currently filtered entities
        Unhook();
        // Now it's safe to reset the filter
        m_Filter.reset();
    }
    SQMOD_UNUSED_VAR(header);
}

// ------------------------------------------------------------------------------------------------
template < class T > void GlobalFilter< T >::Flip(SQInt32 header) noexcept
{
    // Make sure the filter is compatible with the parent event type
    if (!EntType::InEvent(m_Event->m_Type))
    {
        LogWrn("Attempting to <flip %s filter> using an incompatible event type: %s", \
                EntType::Name, GetEventName(m_Event->m_Type));
    }
    // Now it's safe to proceed with reversing the filters
    else
    {
        // Unhook from the currently filtered entities
        Unhook();
        // Reverse the values in the internal filter
        m_Filter.flip();
        // Hook from the newly filtered entities
        Hook();
    }
    SQMOD_UNUSED_VAR(header);
}

// ------------------------------------------------------------------------------------------------
template < class T > void GlobalFilter< T >::Release(SQInt32 id) noexcept
{
    // Do we have to notify someone that this entity is about to be released?
    if (!m_Event->m_OnRelease.IsNull())
    {
        m_Event->m_OnRelease.Execute< SQInt32 >(id);
    }
    // Now disable the entity in the filter
    m_Filter.set(id, false);
}

// ------------------------------------------------------------------------------------------------
template < class T > void GlobalFilter< T >::Hook() noexcept
{
    // Make sure the filter is unaware of the destroy event before proceeeding
    if (EntType::DestroyEvID == m_Event->m_Type)
    {
        return;
    }
    // No iterators here because we're dealing with a bitset!
    unsigned i = 0;
    bool enabled = RefType::Verify(i);
    for (; i < m_Filter.size(); enabled = RefType::Verify(++i))
    {
        // If this bit is enabled then this entity is included so we must hook to it
        if (m_Filter[i] && enabled)
        {
            RefType::Get(i).Destroyed().template Connect< GlobalFilter< T >, &GlobalFilter< T >::Destroyed >(this);
        }
        // If this entity is not active then disable it from the filter as well
        else if (!enabled)
        {
            m_Filter.set(i, false);
        }
    }
}

// ------------------------------------------------------------------------------------------------
template < class T > void GlobalFilter< T >::Unhook() noexcept
{
    // Make sure the filter is unaware of the destroy event before proceeeding
    if (EntType::DestroyEvID == m_Event->m_Type)
    {
        return;
    }
    // No iterators here because we're dealing with a bitset!
    unsigned i = 0;
    bool enabled = RefType::Verify(i);
    for (; i < m_Filter.size(); enabled = RefType::Verify(++i))
    {
        // If this bit is enabled then this entity is included so we must unhook from it
        if (m_Filter[i] && enabled)
        {
            RefType::Get(i).Destroyed().template Disconnect< GlobalFilter< T >, &GlobalFilter< T >::Destroyed >(this);
        }
        // If this entity is not active then disable it from the filter as well
        else if (!enabled)
        {
            m_Filter.set(i, false);
        }
    }
}

} // Namespace:: SqMod

#endif // _SQMOD_EVENT_GLOBAL_HPP_
