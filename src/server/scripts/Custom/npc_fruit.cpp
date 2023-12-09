#include "SmartAI.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "PacmanMap.h"

class npc_fruit : public CreatureScript
{
public:
    npc_fruit() : CreatureScript("npc_fruit") { }

    struct npc_fruitAI : public ScriptedAI {

        npc_fruitAI(Creature* creature) : ScriptedAI(creature) {}

        void InitializeAI() override {
            me->CastSpell(nullptr, PacMan::SPELL_NPC_TELEPORT_VISUAL);
            events.ScheduleEvent(PacMan::FRUIT_DESPAWN, PacMan::FRUIT_LIFETIME);
        }

        void DoAction(int32 action) override {
            if (action == PacMan::CREATURE_TOUCHED_BY_PLAYER) {
                Unit* owner = me->GetOwner();
                if (owner)
                    owner->GetAI()->SetGUID(me->GetGUID(), PacMan::MOVE_FRUIT);
                events.Reset();
            }
        }

        void UpdateAI(uint32 diff) override {
            events.Update(diff);

            if (events.ExecuteEvent() == PacMan::FRUIT_DESPAWN) {
                me->KillSelf();
                me->DespawnOrUnsummon(500ms);
            }
        }

    private:
        EventMap events;
    };
    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_fruitAI(creature);
    }
};

void AddSC_npc_fruit()
{
    new npc_fruit();
}
