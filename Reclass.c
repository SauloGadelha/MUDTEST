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

// Convert string to lowercase in-place
static void strlower(char *s)
{
    for (; *s; s++)
        *s = tolower(*s);
}

int get_sphere_by_name(const char *name)
{
    if (!strcmp(name,"earth")) return SPH_EARTH;
    if (!strcmp(name,"fire")) return SPH_FIRE;
    if (!strcmp(name,"water")) return SPH_WATER;
    if (!strcmp(name,"air")) return SPH_AIR;
    if (!strcmp(name,"spirit")) return SPH_SPIRIT;
    if (!strcmp(name,"void")) return SPH_VOID;
    return -1;
}

void reclassreset(CHAR_DATA *ch)
{
    for (int i = 0; i < 6; i++)
        ch->perm_stat[i] = pc_race_table[ch->race].stats[i];

    int newage = 19;
    int oldperm = ch->pcdata->age_mod_perm;
    ch->pcdata->age_mod_perm = (newage - get_age(ch) + oldperm);

    ch->exp = 1800;
    ch->level = 1;

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

    AFFECT_DATA *af;
    while ((af = ch->affected) != NULL)
        affect_strip(ch, af->type);

    for (int i = 1; i < 65535; i++)
    {
        if ((i >= 248 && i <= 266) || (i >= 281 && i <= 296))
            continue;
        BIT_CLEAR(ch->pcdata->bitptr, i);
    }

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
    ch->pcdata->age_group = AGE_YOUTHFUL;
}

void reclassmsg(CHAR_DATA *ch)
{
    send_to_char("\n\r", ch);
    send_to_char("*********************************** RECLASS ***********************************\n\r", ch);
    send_to_char("\n\r", ch);
    send_to_char("A divine voice whispers in your ears: 'Hero, your efforts have been noticed.'\n\r", ch);
    send_to_char("'As a reward for your unwavering resolve, I shall grant your wish.'\n\r", ch);
    send_to_char("With a violent pull, your spirit is pulled out of your body and made anew.\n\r", ch);
    send_to_char("\n\r", ch);
    send_to_char("*******************************************************************************\n\r", ch);
    send_to_char("\n\r", ch);
}

void do_reclass(CHAR_DATA *ch, char *argument)
{
    if (IS_NPC(ch)) return;

    if (ch->level < 51) {
        send_to_char("You must be level 51 to reclass.\n\r", ch);
        return;
    }

    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH], arg3[MAX_INPUT_LENGTH], msg[MAX_INPUT_LENGTH];
    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
    argument = one_argument(argument, arg3);

    if (arg1[0] == '\0')
    {
        send_to_char("Syntax: reclass <class> [minor sphere] [major sphere]\n\r", ch);
        return;
    }

    strlower(arg1);
    strlower(arg2);
    strlower(arg3);

    // SINGLE-CLASS ASSIGNMENTS
    struct {
        const char *name;
        int class_num;
        int major_sphere;
        int traits[3];
        int traits_count;
    } single_classes[] = {
        {"thief", 12, SPH_THIEF, {TRAIT_FLEET, TRAIT_COWARD, TRAIT_THIEVESCANT}, 3},
        {"bard", 25, SPH_BARD, {0}, 0},
    	{"fighter", 18, SPH_FIGHTER, {TRAIT_PACK_HORSE, TRAIT_SURVIVOR}, 2},
        {"barbarian", 20, SPH_SWORDMASTER, {TRAIT_HOLLOWLEG}, 1},
        {"gladiator", 21, SPH_GLADIATOR, {TRAIT_AMBIDEXTROUS, TRAIT_EXOTICMASTERY}, 2},
        {"swordmaster", 19, SPH_SWORDMASTER, {TRAIT_AMBIDEXTROUS, TRAIT_SWORDMASTERY}, 2},
        {"watcher", 23, SPH_RANGER, {0}, 0},
        {"psionicist", 28, SPH_PSION, {0}, 0},
        {"assassin", 14, SPH_ASSASSIN, {TRAIT_POISONRES}, 1},
        {"bandit", 15, SPH_BANDIT, {TRAIT_PACK_HORSE}, 1},
        {"alchemist", 27, SPH_SWORDMASTER, {TRAIT_MORTICIAN}, 1},
        {"druid", 26, SPH_NATURE, {0}, 0},
    };

    int num_single = sizeof(single_classes)/sizeof(single_classes[0]);
    for (int i = 0; i < num_single; i++)
    {
        if (!strcmp(arg1, single_classes[i].name))
        {
            ch->class_num = single_classes[i].class_num;
            ch->pcdata->major_sphere = single_classes[i].major_sphere;
            group_add(ch, class_table[ch->class_num].base_group, FALSE);
            group_add(ch, class_table[ch->class_num].default_group, FALSE);

            for (int t = 0; t < single_classes[i].traits_count; t++)
                BIT_SET(ch->pcdata->traits, single_classes[i].traits[t]);

            snprintf(msg, sizeof(msg), "You choose to be reborn as a %s. So be it.\n\r", single_classes[i].name);
            send_to_char(msg, ch);

            reclassreset(ch);
            reclassmsg(ch);
            return;
        }
    }

    // TEMPLAR CLASS
    if (!strcmp(arg1,"templar"))
    {
        if (arg2[0] == '\0')
        {
            send_to_char("You must choose a sphere for templar. Choices: earth, fire, water, air, spirit, void.\n\r", ch);
            return;
        }

        if (!strcmp(arg2,"earth"))
            { ch->class_num = 7; ch->pcdata->major_sphere = SPH_EARTH; BIT_SET(ch->pcdata->traits, TRAIT_NIMBLE); }
        else if (!strcmp(arg2,"fire"))
            { ch->class_num = 5; ch->pcdata->major_sphere = SPH_FIRE; BIT_SET(ch->pcdata->traits, TRAIT_SURVIVOR); BIT_SET(ch->pcdata->traits, TRAIT_NIMBLE); }
        else if (!strcmp(arg2,"water"))
            { ch->class_num = 6; ch->pcdata->major_sphere = SPH_WATER; BIT_SET(ch->pcdata->traits, TRAIT_ENDURANCE); BIT_SET(ch->pcdata->traits, TRAIT_NIMBLE); }
        else if (!strcmp(arg2,"air"))
            { ch->class_num = 10; ch->pcdata->major_sphere = SPH_AIR; BIT_SET(ch->pcdata->traits, TRAIT_NIMBLE); }
        else if (!strcmp(arg2,"spirit"))
            { ch->class_num = 9; ch->pcdata->major_sphere = SPH_SPIRIT; BIT_SET(ch->pcdata->traits, TRAIT_BRAVE); }
        else if (!strcmp(arg2,"void"))
            { ch->class_num = 8; ch->pcdata->major_sphere = SPH_VOID; BIT_SET(ch->pcdata->traits, TRAIT_OBSCURE); }
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

    // SCHOLAR CLASS
    if (!strcmp(arg1,"scholar"))
    {
        if (arg2[0] == '\0' || arg3[0] == '\0')
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

    send_to_char("Syntax: reclass <class name> [minor sphere] [major sphere]\n\r", ch);
}
