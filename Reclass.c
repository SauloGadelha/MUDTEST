#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <cmath>
#include "merc.h"
#include "interp.h"

// Sphere lookup function
int get_sphere_by_name(const char *name)
{
    if (!strcmp(name, "earth")) return SPH_EARTH;
    if (!strcmp(name, "fire")) return SPH_FIRE;
    if (!strcmp(name, "water")) return SPH_WATER;
    if (!strcmp(name, "air")) return SPH_AIR;
    if (!strcmp(name, "spirit")) return SPH_SPIRIT;
    if (!strcmp(name, "void")) return SPH_VOID;
    return -1;
}

// Reset character to default stats and apply bonuses
void reclassreset(CHAR_DATA *ch)
{
    // Reset base stats to race defaults
    for (int i = 0; i < 6; i++)
        ch->perm_stat[i] = pc_race_table[ch->race].stats[i];

    // Reset Age
    int newage = 19;
    int oldperm = ch->pcdata->age_mod_perm;
    ch->pcdata->age_mod_perm = (newage - get_age(ch) + oldperm);

    // Reset Exp and Level
    ch->exp = 1800;
    ch->level = 1;

    // Strip objects and affects
    OBJ_DATA *obj, *obj_next;
    int affmod = (is_affected(ch, gsn_affinity)) ? get_modifier(ch->affected, gsn_affinity) : -1;

    for (obj = ch->carrying; obj != NULL; obj = obj_next)
    {
        bool floating = FALSE;
        obj_next = obj->next_content;

        if (IS_SET(obj->worn_on, WORN_FLOAT))
            floating = TRUE;

        obj_from_char(obj);
        if (!IS_VALID(obj)) continue;

        if (obj->item_type == ITEM_POTION)
            obj->timer = number_range(500, 1000);

        if (obj->item_type == ITEM_SCROLL)
            obj->timer = number_range(1000, 2500);

        if (floating)
        {
            act("$p falls to the floor.", ch, obj, NULL, TO_ROOM);
            obj_to_room(obj, ch->in_room);
        }
        else
        {
            if (IS_SET(obj->wear_flags, ITEM_WEAR_SIGIL)
                || IS_SET(obj->wear_flags, ITEM_PROG)
                || ((obj->extra_flags[0] & ITEM_AFFINITY)
                    && (get_modifier(obj->affected, gsn_affinity) == affmod)))
            {
                obj_to_char(obj, ch);
            }
            else
            {
                obj_to_room(obj, ch->in_room);
            }
        }
    }

    // Remove all affects
    AFFECT_DATA *af;
    while ((af = ch->affected) != NULL)
        affect_strip(ch, af->type);

    // Clear most bitflags except protected ranges
    for (int i = 1; i < 65535; i++)
    {
        if ((i >= 248 && i <= 266) || (i >= 281 && i <= 296))
            continue;
        BIT_CLEAR(ch->pcdata->bitptr, i);
    }

    // Apply 5% bonus to max stats
    int BonusHit  = std::max(1, (int)round(ch->max_hit  * 0.05));
    int BonusMana = std::max(1, (int)round(ch->max_mana * 0.05));
    int BonusMove = std::max(1, (int)round(ch->max_move * 0.05));

    ch->max_hit  += BonusHit;
    ch->max_mana += BonusMana;
    ch->max_move += BonusMove;

    ch->pcdata->perm_hit  = ch->max_hit;
    ch->pcdata->perm_mana = ch->max_mana;
    ch->pcdata->perm_move = ch->max_move;

    ch->pcdata->death_count = 0;
    ch->pcdata->age_group = AGE_YOUTHFUL;
}

// Display reclass message
void reclassmsg(CHAR_DATA *ch)
{
    send_to_char("\n\r", ch);
    send_to_char("*********************************** RECLASS ***********************************\n\r", ch);
    send_to_char("\n\r", ch);
    send_to_char("A divine voice whispers in your ears: 'Hero, your efforts have been noticed.'\n\r", ch);
    send_to_char("'As a reward for your unwavering resolve, I shall grant your wish.'\n\r", ch);
    send_to_char("With a violent pull, your spirit is drawn out of your body and made anew.\n\r", ch);
    send_to_char("\n\r", ch);
    send_to_char("*******************************************************************************\n\r", ch);
    send_to_char("\n\r", ch);
}

// Single function to handle reclassing
void do_reclass(CHAR_DATA *ch, char *arg1, char *arg2, char *arg3)
{
    if (IS_NPC(ch)) return;

    if (ch->level < 51)
    {
        send_to_char("You must be level 51 to reclass.\n\r", ch);
        return;
    }

    if (!arg1 || strlen(arg1) == 0)
    {
        send_to_char("Syntax: reclass <class> [minor sphere] [major sphere]\n\r", ch);
        return;
    }

    // Table-driven class data
    struct CLASS_DATA {
        const char *name;
        int class_num;
        int major_sphere;
        int minor_sphere;
        int traits[3];
    } class_table_data[] = {
        {"thief", 12, SPH_THIEF, -1, {TRAIT_FLEET, TRAIT_COWARD, TRAIT_THIEVESCANT}},
        {"bard", 25, SPH_BARD, -1, {0,0,0}},
        {"barbarian", 20, SPH_BARBARIAN, -1, {TRAIT_HOLLOWLEG,0,0}},
        {"gladiator", 21, SPH_GLADIATOR, -1, {TRAIT_AMBIDEXTROUS, TRAIT_EXOTICMASTERY,0}},
        {"swordmaster", 19, SPH_SWORDMASTER, -1, {TRAIT_AMBIDEXTROUS, TRAIT_SWORDMASTERY,0}},
        {"ranger", 23, SPH_RANGER, -1, {0,0,0}},
        {"psionicist", 28, SPH_PSION, -1, {0,0,0}},
        {"assassin", 14, SPH_ASSASSIN, -1, {TRAIT_POISONRES,0,0}},
        {"bandit", 15, SPH_BANDIT, -1, {TRAIT_PACK_HORSE,0,0}},
        {"alchemist", 27, SPH_SWORDMASTER, -1, {TRAIT_MORTICIAN,0,0}},
        {"druid", 26, SPH_NATURE, -1, {0,0,0}},
        {"scholar", 0, -1, -1, {TRAIT_MAGAPT,0,0}}
    };

    // Handle single-class assignments
    for (size_t i = 0; i < sizeof(class_table_data)/sizeof(class_table_data[0]); i++)
    {
        if (!strcmp(arg1, class_table_data[i].name))
        {
            // Scholar is multi-sphere, handled separately
            if (!strcmp(arg1,"scholar"))
                break;

            ch->class_num = class_table_data[i].class_num;
            ch->pcdata->major_sphere = class_table_data[i].major_sphere;
            group_add(ch, class_table[ch->class_num].base_group, FALSE);
            group_add(ch, class_table[ch->class_num].default_group, FALSE);

            for (int t = 0; t < 3; t++)
                if (class_table_data[i].traits[t])
                    BIT_SET(ch->pcdata->traits, class_table_data[i].traits[t]);

            char buf[256];
            snprintf(buf, sizeof(buf), "You choose to be reborn as a %s. So be it.\n\r", arg1);
            send_to_char(buf, ch);

            reclassreset(ch);
            reclassmsg(ch);
            return;
        }
    }

    // Templar class handling
    if (!strcmp(arg1,"templar"))
    {
        if (!arg2 || strlen(arg2) == 0)
        {
            send_to_char("You must choose a sphere for templar. Choices: earth, fire, water, air, spirit, void.\n\r", ch);
            return;
        }

        if (!strcmp(arg2,"earth"))      { ch->class_num = 7;  ch->pcdata->major_sphere = SPH_EARTH; BIT_SET(ch->pcdata->traits, TRAIT_NIMBLE); }
        else if (!strcmp(arg2,"fire"))  { ch->class_num = 5;  ch->pcdata->major_sphere = SPH_FIRE; BIT_SET(ch->pcdata->traits, TRAIT_SURVIVOR); BIT_SET(ch->pcdata->traits, TRAIT_NIMBLE); }
        else if (!strcmp(arg2,"water")) { ch->class_num = 6;  ch->pcdata->major_sphere = SPH_WATER; BIT_SET(ch->pcdata->traits, TRAIT_ENDURANCE); BIT_SET(ch->pcdata->traits, TRAIT_NIMBLE); }
        else if (!strcmp(arg2,"air"))   { ch->class_num = 10; ch->pcdata->major_sphere = SPH_AIR; BIT_SET(ch->pcdata->traits, TRAIT_NIMBLE); }
        else if (!strcmp(arg2,"spirit")){ ch->class_num = 9;  ch->pcdata->major_sphere = SPH_SPIRIT; BIT_SET(ch->pcdata->traits, TRAIT_BRAVE); }
        else if (!strcmp(arg2,"void"))  { ch->class_num = 8;  ch->pcdata->major_sphere = SPH_VOID; BIT_SET(ch->pcdata->traits, TRAIT_OBSCURE); }
        else
        {
            send_to_char("Invalid sphere for templar. Choices: earth, fire, water, air, spirit, void.\n\r", ch);
            return;
        }

        group_add(ch, class_table[ch->class_num].base_group, FALSE);
        group_add(ch, class_table[ch->class_num].default_group, FALSE);
        ch->pcdata->learned[gsn_language_arcane] = 100;
        send_to_char("You have chosen to be reborn as a templar. So be it.\n\r", ch);
        reclassreset(ch);
        reclassmsg(ch);
        return;
    }

    // Scholar class handling
    if (!strcmp(arg1,"scholar"))
    {
        if (!arg2 || !arg3)
        {
            send_to_char("You must choose both major and minor spheres for scholar. Choices: earth, fire, water, air, spirit, void.\n\r", ch);
            return;
        }

        ch->class_num = 0;
        ch->pcdata->major_sphere = get_sphere_by_name(arg2);
        ch->pcdata->minor_sphere = get_sphere_by_name(arg3);

        if (ch->pcdata->major_sphere < 0 || ch->pcdata->minor_sphere < 0)
        {
            send_to_char("Invalid major or minor sphere. Choices: earth, fire, water, air, spirit, void.\n\r", ch);
            return;
        }

        group_add(ch, class_table[ch->class_num].base_group, FALSE);
        group_add(ch, class_table[ch->class_num].default_group, FALSE);
        ch->pcdata->learned[gsn_language_arcane] = 100;
        BIT_SET(ch->pcdata->traits, TRAIT_MAGAPT);

        send_to_char("You have chosen to be reborn as a scholar. So be it.\n\r", ch);
        reclassreset(ch);
        reclassmsg(ch);
        return;
    }

    send_to_char("Syntax: reclass <class name> <minor sphere> <major sphere>\n\r", ch);
}
