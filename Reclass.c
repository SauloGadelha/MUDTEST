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

int get_sphere_by_name(const char *name)
{
    if (!strcmp(name,"earth"))  return SPH_EARTH;
    if (!strcmp(name,"fire"))   return SPH_FIRE;
    if (!strcmp(name,"water"))  return SPH_WATER;
    if (!strcmp(name,"air"))    return SPH_AIR;
    if (!strcmp(name,"spirit")) return SPH_SPIRIT;
    if (!strcmp(name,"void"))   return SPH_VOID;
    return -1;
}

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
                obj_to_room(obj, ch->in_room);
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

    // Incremental bonus based on current max stats
    int BonusHit  = (int)round(ch->max_hit  * 0.05);
    int BonusMana = (int)round(ch->max_mana * 0.05);
    int BonusMove = (int)round(ch->max_move * 0.05);

    if (BonusHit < 1) BonusHit = 1;
    if (BonusMana < 1) BonusMana = 1;
    if (BonusMove < 1) BonusMove = 1;

    ch->max_hit  += BonusHit;
    ch->max_mana += BonusMana;
    ch->max_move += BonusMove;

    ch->pcdata->perm_hit  = ch->max_hit;
    ch->pcdata->perm_mana = ch->max_mana;
    ch->pcdata->perm_move = ch->max_move;

    ch->pcdata->death_count = 0;
    ch->pcdata->age_group   = AGE_YOUTHFUL;
}

void reclassmsg(CHAR_DATA *ch)
{
    send_to_char("\n\r", ch);
    send_to_char("*********************************** RECLASS ***********************************\n\r", ch);
    send_to_char("\n\r", ch);
    send_to_char("A divine voice whispers in your ears: 'Heroe, your efforts have been noticed.'\n\r", ch);
    send_to_char("'As a reward for your unwavering resolve, I shall grant your wish.'\n\r", ch);
    send_to_char("With a violent pull, your spirit is pulled out of your body and made anew.\n\r", ch);
    send_to_char("\n\r", ch);
    send_to_char("*******************************************************************************\n\r", ch);
    send_to_char("\n\r", ch);
}

void do_reclass(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch)) return;

    if (ch->level < 51)
    {
        send_to_char("You must be level 51 to reclass.\n\r", ch);
        return;
    }

    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
    argument = one_argument(argument, arg3);

    if (!arg1[0])
    {
        send_to_char("Syntax: reclass <class> [minor sphere] [major sphere]\n\r", ch);
        return;
    }

    // --- SINGLE-CLASS ASSIGNMENTS ---
    if (!strcmp(arg1,"thief"))
    {
        ch->class_num = 12;
        ch->pcdata->major_sphere = SPH_THIEF;
        group_add(ch, class_table[ch->class_num].base_group, FALSE);
        group_add(ch, class_table[ch->class_num].default_group, FALSE);
        send_to_char("You choose to be reborn as a thief. So be it.\n\r", ch);

        BIT_SET(ch->pcdata->traits, TRAIT_FLEET);
        BIT_SET(ch->pcdata->traits, TRAIT_COWARD);
        BIT_SET(ch->pcdata->traits, TRAIT_THIEVESCANT);

        reclassreset(ch);
        reclassmsg(ch);
        return;
    }

    if (!strcmp(arg1,"bard"))
    {
        ch->class_num = 25;
        ch->pcdata->major_sphere = SPH_BARD;
        group_add(ch, class_table[ch->class_num].base_group, FALSE);
        group_add(ch, class_table[ch->class_num].default_group, FALSE);
        send_to_char("You choose to be reborn as a bard. So be it.\n\r", ch);

        reclassreset(ch);
        reclassmsg(ch);
        return;
    }

    if (!strcmp(arg1,"barbarian"))
    {
        ch->class_num = 20;
        ch->pcdata->major_sphere = SPH_BARBARIAN;
        group_add(ch, class_table[ch->class_num].base_group, FALSE);
        group_add(ch, class_table[ch->class_num].default_group, FALSE);
        send_to_char("You choose to be reborn as a barbarian. So be it.\n\r", ch);

        BIT_SET(ch->pcdata->traits, TRAIT_HOLLOWLEG);
        reclassreset(ch);
        reclassmsg(ch);
        return;
    }

    // --- Repeat same pattern for other single classes ---
    // ... swordmaster, gladiator, ranger, psionicist, assassin, bandit, alchemist, druid ...
    // Keep your original logic here for brevity.

    // --- TEMPLAR CLASS ---
    if (!strcmp(arg1,"templar"))
    {
        if (!arg2[0])
        {
            send_to_char("You must choose a sphere for templar. Choices: earth, fire, water, air, spirit, void.\n\r", ch);
            return;
        }

        if (!strcmp(arg2,"earth"))  { ch->class_num = 7;  ch->pcdata->major_sphere = SPH_EARTH;  BIT_SET(ch->pcdata->traits, TRAIT_NIMBLE); }
        else if (!strcmp(arg2,"fire"))  { ch->class_num = 5;  ch->pcdata->major_sphere = SPH_FIRE;  BIT_SET(ch->pcdata->traits, TRAIT_SURVIVOR); BIT_SET(ch->pcdata->traits, TRAIT_NIMBLE); }
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

    // --- SCHOLAR CLASS ---
    if (!strcmp(arg1,"scholar"))
    {
        if (!arg2[0] || !arg3[0])
        {
            send_to_char("You must choose both major and minor spheres for scholar. Choices: earth, fire, water, air, spirit, void.\n\r", ch);
            return;
        }

        ch->class_num = 0; // default scholar class number
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
