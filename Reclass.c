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

void reclassmsg( CHAR_DATA *ch )
{
	send_to_char( "\n\r", ch );
	send_to_char( "*********************************** RECLASS ***********************************\n\r", ch );
	send_to_char( "\n\r", ch );
	send_to_char( "A divine voice whispers in your ears: 'Heroe, your efforts have been noticed.'.\n\r", ch );
	send_to_char( "'As a reward for your unwavering resolve, I shall grant your wish.''.\n\r", ch );
	send_to_char( "With a violent pull, your spirit is pulled out of your body and made anew.\n\r", ch );
	send_to_char( "\n\r", ch );
	send_to_char( "*******************************************************************************\n\r", ch );
	send_to_char( "\n\r", ch );
}

void do_reclass( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH],arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );
	
	if (IS_NPC(ch)) return;
	
	if (ch->level < 51) {
		send_to_char( "You must be level 51 to reclass.\n\r", ch );
		return;
	}
	
	int i;
	int BonusHit;
	int BonusMana;
	int BonusMove;
	
	if (!strcmp(arg1,"fighter") || !strcmp(arg1,"watcher") || !strcmp(arg1,"thief") || !strcmp(arg1,"bard") 
	|| !strcmp(arg1,"barbarian") || !strcmp(arg1,"gladiator") || !strcmp(arg1,"swordmaster") || !strcmp(arg1,"ranger")
	|| !strcmp(arg1,"psionicist") || !strcmp(arg1,"druid") || !strcmp(arg1,"assassin") || !strcmp(arg1,"bandit") ||  
	!strcmp(arg1,"alchemist") || !strcmp(arg1,"templar") && (!strcmp(arg2,"earth") || !strcmp(arg2,"fire") || 
	!strcmp(arg2,"water") || !strcmp(arg2,"air") || !strcmp(arg2,"spirit") || !strcmp(arg2,"void")) || !strcmp(arg1,"scholar") 
	&& (!strcmp(arg2,"earth") || !strcmp(arg2,"fire") || !strcmp(arg2,"water") || !strcmp(arg2,"air") || !strcmp(arg2,"spirit") 
	|| !strcmp(arg2,"void")) && (!strcmp(arg3,"earth") || !strcmp(arg3,"fire") || !strcmp(arg3,"water") || !strcmp(arg3,"air") || 
	!strcmp(arg3,"spirit") || !strcmp(arg3,"void"))) 
	
	{
	
		for (i = 0; i < 6; i++)
		ch->perm_stat[i] = pc_race_table[ch->race].stats[i];
		
		int newage, oldperm;

	    newage = 19;
        oldperm = ch->pcdata->age_mod_perm;
	    ch->pcdata->age_mod_perm = (newage - get_age(ch) + oldperm);

		
		ch->exp     = 1800;
		ch->level = 1;
	
		//Stripping char
		
		OBJ_DATA *obj;
		OBJ_DATA *obj_next;

		int affmod;
		
		if (is_affected(ch, gsn_affinity))
		affmod = get_modifier(ch->affected, gsn_affinity);
		else affmod = -1;
		
		for ( obj = ch->carrying; obj != NULL; obj = obj_next )
		{
			bool floating = FALSE;
			obj_next = obj->next_content;
		 
			if (IS_SET(obj->worn_on, WORN_FLOAT))
				floating = TRUE;

			obj_from_char(obj);
			if (!IS_VALID(obj))
				continue;

			if (obj->item_type == ITEM_POTION)
				obj->timer = number_range(500,1000);

			if (obj->item_type == ITEM_SCROLL)
				obj->timer = number_range(1000,2500);

			if (floating)
			{
				act("$p falls to the floor.",ch,obj,NULL,TO_ROOM);
				obj_to_room(obj,ch->in_room);
			}
			else
			{
				if (IS_SET(obj->wear_flags, ITEM_WEAR_SIGIL)
				 || IS_SET(obj->wear_flags, ITEM_PROG)
				 || ((obj->extra_flags[0] & ITEM_AFFINITY)
				  && (get_modifier(obj->affected, gsn_affinity) == affmod)))
					obj_to_char(obj, ch);
				else
					obj_to_room(obj,ch->in_room);                
			}
		}
		
		//Stripping Affects
		
		AFFECT_DATA *af;

		if (IS_NAFFECTED(ch, AFF_FLESHTOSTONE) || IS_OAFFECTED(ch, AFF_ENCASE))
	    REMOVE_BIT(ch->act, PLR_FREEZE);

        while ((af = ch->affected) != NULL)
            affect_strip(ch,af->type);

		//Clearing Bits
		
		
		
		for (int i = 1; i < 65535; i++ ) 
		{
			if (((i >= 248) && (i <= 266)) //BloodPyre Bits
			|| ((i >= 281) && (i <= 296))) //Rune Bits
			continue;
			else
				BIT_CLEAR(ch->pcdata->bitptr, i);
		}

		
				
		for (int i = 1; i < 65535; i++ ) {
			if ((i < 281) || (i > 296))
			BIT_CLEAR(ch->pcdata->bitptr, i);
		}
		
		
	
		//Apply Bonus
		
		BonusHit = round(ch->max_hit / 200);
		BonusMana = round(ch->max_mana / 200);	
		BonusMove = round(ch->max_move / 200);
		if (BonusHit < 0) BonusHit = 0;
		if (BonusMana < 0) BonusHit = 0;
		if (BonusMove < 0) BonusHit = 0;
				
		ch->max_hit	= 100 + BonusHit;
		ch->max_mana = 100 + BonusMana;
		ch->max_move = 100 + BonusMove;
		ch->pcdata->perm_hit	= 100 + BonusHit;
           ch->pcdata->perm_mana   = 100 + BonusMana;
           ch->pcdata->perm_move   = 100 + BonusMove;
		
		ch->pcdata->death_count = 0;
	
		ch->pcdata->age_group = AGE_YOUTHFUL;

	}

	
	if (!strcmp(arg1,"fighter")) 
    {
		ch->class_num = 18;
		ch->pcdata->major_sphere = SPH_SWORDMASTER;
		group_add(ch, class_table[ch->class_num].base_group, FALSE);
		group_add(ch, class_table[ch->class_num].default_group, FALSE);
        send_to_char( "You choose to be reborn as a fighter. So be it.\n\r", ch );
		reclassmsg(ch);
        return;
    }
	
	if (!strcmp(arg1,"watcher"))
    {
		ch->class_num = 13;
		ch->pcdata->major_sphere = SPH_SWORDMASTER;
		group_add(ch, class_table[ch->class_num].base_group, FALSE);
		group_add(ch, class_table[ch->class_num].default_group, FALSE);
        send_to_char( "You choose to be reborn as a watcher. So be it.\n\r", ch );
		
		BIT_SET(ch->pcdata->traits, TRAIT_LIGHTSLEEPER);
	    BIT_SET(ch->pcdata->traits, TRAIT_EAGLE_EYED);
		reclassmsg(ch);
        return;
    }
	
		if (!strcmp(arg1,"thief"))
    {
		ch->class_num = 12;
		ch->pcdata->major_sphere = SPH_SWORDMASTER;
		group_add(ch, class_table[ch->class_num].base_group, FALSE);
		group_add(ch, class_table[ch->class_num].default_group, FALSE);
        send_to_char( "You choose to be reborn as a thief. So be it.\n\r", ch );
		
		BIT_SET(ch->pcdata->traits, TRAIT_FLEET);
	    BIT_SET(ch->pcdata->traits, TRAIT_COWARD);
	    BIT_SET(ch->pcdata->traits, TRAIT_THIEVESCANT);
		reclassmsg(ch);
        return;
    }
	
	if (!strcmp(arg1,"bard")) 
    {
		ch->class_num = 25;
		ch->pcdata->major_sphere = SPH_SWORDMASTER;
		group_add(ch, class_table[ch->class_num].base_group, FALSE);
		group_add(ch, class_table[ch->class_num].default_group, FALSE);
        send_to_char( "You choose to be reborn as a bard. So be it.\n\r", ch );
		reclassmsg(ch);
	    return;
    }

	if (!strcmp(arg1,"barbarian")) 
    {
		ch->class_num = 20;
		ch->pcdata->major_sphere = SPH_SWORDMASTER;
		group_add(ch, class_table[ch->class_num].base_group, FALSE);
		group_add(ch, class_table[ch->class_num].default_group, FALSE);
        send_to_char( "You choose to be reborn as a barbarian. So be it.\n\r", ch );
		
		BIT_SET(ch->pcdata->traits, TRAIT_HOLLOWLEG);
		reclassmsg(ch);
        return;
    }

	if (!strcmp(arg1,"gladiator"))
    {
		ch->class_num = 21;
		ch->pcdata->major_sphere = SPH_SWORDMASTER;
		group_add(ch, class_table[ch->class_num].base_group, FALSE);
		group_add(ch, class_table[ch->class_num].default_group, FALSE);
        send_to_char( "You choose to be reborn as a gladiator. So be it.\n\r", ch );
		
		BIT_SET(ch->pcdata->traits, TRAIT_AMBIDEXTROUS);
	    BIT_SET(ch->pcdata->traits, TRAIT_EXOTICMASTERY);
		reclassmsg(ch);
        return;
    }

	if (!strcmp(arg1,"swordmaster"))
    {
        ch->class_num = 19;
		ch->pcdata->major_sphere = SPH_SWORDMASTER;
		group_add(ch, class_table[ch->class_num].base_group, FALSE);
		group_add(ch, class_table[ch->class_num].default_group, FALSE);
		send_to_char( "You choose to be reborn as a swordmaster. So be it.\n\r", ch );
		
		BIT_SET(ch->pcdata->traits, TRAIT_AMBIDEXTROUS);
		BIT_SET(ch->pcdata->traits, TRAIT_SWORDMASTERY);
		reclassmsg(ch);
        return;
    }

	if (!strcmp(arg1,"ranger"))
    {
		ch->class_num = 23;
		ch->pcdata->major_sphere = SPH_SWORDMASTER;
		group_add(ch, class_table[ch->class_num].base_group, FALSE);
		group_add(ch, class_table[ch->class_num].default_group, FALSE);
        send_to_char( "You choose to be reborn as a ranger. So be it.\n\r", ch );
		reclassmsg(ch);
        return;
    }

	if (!strcmp(arg1,"psionicist"))
    {
		ch->class_num = 28;
		ch->pcdata->major_sphere = SPH_SWORDMASTER;
		group_add(ch, class_table[ch->class_num].base_group, FALSE);
		group_add(ch, class_table[ch->class_num].default_group, FALSE);
        send_to_char( "You choose to be reborn as a psionicist. So be it.\n\r", ch );
		reclassmsg(ch);
        return;
    }

	if (!strcmp(arg1,"assassin"))
    {
		ch->class_num = 14;
		ch->pcdata->major_sphere = SPH_SWORDMASTER;
		group_add(ch, class_table[ch->class_num].base_group, FALSE);
		group_add(ch, class_table[ch->class_num].default_group, FALSE);
        send_to_char( "You choose to be reborn as an assassin. So be it.\n\r", ch );
		
		BIT_SET(ch->pcdata->traits, TRAIT_POISONRES);
		reclassmsg(ch);
        return;
    }
	
		if (!strcmp(arg1,"bandit"))
    {
		ch->class_num = 15;
		ch->pcdata->major_sphere = SPH_SWORDMASTER;
		group_add(ch, class_table[ch->class_num].base_group, FALSE);
		group_add(ch, class_table[ch->class_num].default_group, FALSE);
        send_to_char( "You choose to be reborn as a bandit. So be it.\n\r", ch );
		
		BIT_SET(ch->pcdata->traits, TRAIT_PACK_HORSE);
		reclassmsg(ch);
        return;
    }

	if (!strcmp(arg1,"alchemist"))
    {
		ch->class_num = 27;
		ch->pcdata->major_sphere = SPH_SWORDMASTER;
		group_add(ch, class_table[ch->class_num].base_group, FALSE);
		group_add(ch, class_table[ch->class_num].default_group, FALSE);
        send_to_char( "You choose to be reborn as an alchemist. So be it.\n\r", ch );
		
		BIT_SET(ch->pcdata->traits, TRAIT_MORTICIAN);
		reclassmsg(ch);
        return;
    }

	if (!strcmp(arg1,"druid"))
    {
		ch->class_num = 27;
		ch->pcdata->major_sphere = SPH_SWORDMASTER;
		group_add(ch, class_table[ch->class_num].base_group, FALSE);
		group_add(ch, class_table[ch->class_num].default_group, FALSE);
        send_to_char( "You choose to be reborn as a druid. So be it.\n\r", ch );
		
		send_to_char( "You choose to be reborn as a druid. So be it.\n\r", ch );
		reclassmsg(ch);
        return;
    }

	if (!strcmp(arg1,"templar"))
	{
		
		if (!strcmp(arg2,"earth"))
		{
			ch->class_num = 7;
			ch->pcdata->major_sphere = SPH_EARTH;
			group_add(ch, class_table[ch->class_num].base_group, FALSE);
			group_add(ch, class_table[ch->class_num].default_group, FALSE);
			ch->pcdata->learned[gsn_language_arcane] = 100;
			send_to_char( "You choose to be reborn as an earth templar. So be it.\n\r", ch );
			
			BIT_SET(ch->pcdata->traits, TRAIT_NIMBLE);
			reclassmsg(ch);
			return;
		}

		if ( !strcmp(arg2,"fire"))
		{
			ch->class_num = 5;
			ch->pcdata->major_sphere = SPH_FIRE;
			group_add(ch, class_table[ch->class_num].base_group, FALSE);
			group_add(ch, class_table[ch->class_num].default_group, FALSE);
			ch->pcdata->learned[gsn_language_arcane] = 100;
			send_to_char( "You choose to be reborn as a fire templar. So be it.\n\r", ch );
			
			BIT_SET(ch->pcdata->traits, TRAIT_SURVIVOR);
			BIT_SET(ch->pcdata->traits, TRAIT_NIMBLE);
			reclassmsg(ch);
			return;
		}
		
		if ( !strcmp(arg2,"water"))
		{
			ch->class_num = 6;
			ch->pcdata->major_sphere = SPH_WATER;
			group_add(ch, class_table[ch->class_num].base_group, FALSE);
			group_add(ch, class_table[ch->class_num].default_group, FALSE);
			ch->pcdata->learned[gsn_language_arcane] = 100;
			send_to_char( "You choose to be reborn as a water templar. So be it.\n\r", ch );
			
			BIT_SET(ch->pcdata->traits, TRAIT_ENDURANCE);
			BIT_SET(ch->pcdata->traits, TRAIT_NIMBLE);
			reclassmsg(ch);
			return;
		}
		
		if ( !strcmp(arg2,"air"))
		{
			ch->class_num = 10;
			ch->pcdata->major_sphere = SPH_AIR;
			group_add(ch, class_table[ch->class_num].base_group, FALSE);
			group_add(ch, class_table[ch->class_num].default_group, FALSE);
			ch->pcdata->learned[gsn_language_arcane] = 100;
			send_to_char( "You choose to be reborn as an air templar. So be it.\n\r", ch );
			
			BIT_SET(ch->pcdata->traits, TRAIT_NIMBLE);
			reclassmsg(ch);
			return;
		}
		
		if ( !strcmp(arg2,"spirit"))
		{
			ch->class_num = 9;
			ch->pcdata->major_sphere = SPH_SPIRIT;
			group_add(ch, class_table[ch->class_num].base_group, FALSE);
			group_add(ch, class_table[ch->class_num].default_group, FALSE);
			ch->pcdata->learned[gsn_language_arcane] = 100;
			send_to_char( "You choose to be reborn as a spirit templar. So be it.\n\r", ch );
			
			BIT_SET(ch->pcdata->traits, TRAIT_BRAVE);
			reclassmsg(ch);
			return;
		}
		
		if ( !strcmp(arg2,"void"))
		{
			ch->class_num = 8;
			ch->pcdata->major_sphere = SPH_VOID;
			group_add(ch, class_table[ch->class_num].base_group, FALSE);
			group_add(ch, class_table[ch->class_num].default_group, FALSE);
			ch->pcdata->learned[gsn_language_arcane] = 100;
			send_to_char( "You choose to be reborn as a void templar. So be it.\n\r", ch );
			
			BIT_SET(ch->pcdata->traits, TRAIT_OBSCURE);
			reclassmsg(ch);
			return;
		}
	
	send_to_char( "You must choose an appropriate sphere for the templar class. Avaliable choices are: earth, fire, water, air, void and spirit.\n\r", ch );
    return;
	}
	
	if (!strcmp(arg1,"scholar"))
	{
		
		if ( !strcmp(arg2,"earth"))
		{
			if ( !strcmp(arg3,"earth"))
			{
				ch->class_num = 1;
				ch->pcdata->major_sphere = SPH_EARTH;
				ch->pcdata->minor_sphere = SPH_EARTH;
				group_add(ch, class_table[ch->class_num].base_group, FALSE);
				group_add(ch, class_table[ch->class_num].default_group, FALSE);
				ch->pcdata->learned[gsn_language_arcane] = 100;
				BIT_SET(ch->pcdata->traits, TRAIT_MAGAPT);
				send_to_char( "You choose to be reborn as a greater scholar of the earth sphere. So be it.\n\r", ch );
				reclassmsg(ch);
				return;
			}
			
			if ( !strcmp(arg3,"fire"))
			{
				ch->class_num = 1;
				group_add(ch, "minor fire v2", FALSE);
				group_add(ch, "mixed earth fire", false);
				ch->pcdata->major_sphere = SPH_EARTH;
				ch->pcdata->minor_sphere = SPH_FIRE;
				group_add(ch, class_table[ch->class_num].base_group, FALSE);
				group_add(ch, class_table[ch->class_num].default_group, FALSE);
				ch->pcdata->learned[gsn_language_arcane] = 100;	
				BIT_SET(ch->pcdata->traits, TRAIT_MAGAPT);
				send_to_char( "You choose to be reborn as a scholar of the earth/fire spheres. So be it.\n\r", ch );
				reclassmsg(ch);
				return;
			}
		
			if ( !strcmp(arg3,"water"))
			{
				ch->class_num = 1;
				ch->pcdata->major_sphere = SPH_EARTH;
				ch->pcdata->minor_sphere = SPH_WATER;
				group_add(ch, "minor water v2", FALSE);
				group_add(ch, "mixed earth water", false);
				group_add(ch, class_table[ch->class_num].base_group, FALSE);
				group_add(ch, class_table[ch->class_num].default_group, FALSE);
				ch->pcdata->learned[gsn_language_arcane] = 100;	
				BIT_SET(ch->pcdata->traits, TRAIT_MAGAPT);
				send_to_char( "You choose to be reborn as a scholar of the earth/water spheres. So be it.\n\r", ch );
				reclassmsg(ch);
				return;
			}
		
			if ( !strcmp(arg3,"air"))
			{
				ch->class_num = 1;
				ch->pcdata->major_sphere = SPH_EARTH;
				ch->pcdata->minor_sphere = SPH_AIR;
				group_add(ch, "minor air v2", FALSE);
				group_add(ch, "mixed earth air", false);
				group_add(ch, class_table[ch->class_num].base_group, FALSE);
				group_add(ch, class_table[ch->class_num].default_group, FALSE);
				ch->pcdata->learned[gsn_language_arcane] = 100;	
				BIT_SET(ch->pcdata->traits, TRAIT_MAGAPT);
				send_to_char( "You choose to be reborn as a scholar of the earth/air spheres. So be it.\n\r", ch );
				reclassmsg(ch);
				return;
			}
		
			if ( !strcmp(arg3,"spirit"))
			{
				ch->class_num = 1;
				ch->pcdata->major_sphere = SPH_EARTH;
				ch->pcdata->minor_sphere = SPH_SPIRIT;
				group_add(ch, "minor spirit v2", FALSE);
				group_add(ch, "mixed earth spirit", false);
				group_add(ch, class_table[ch->class_num].base_group, FALSE);
				group_add(ch, class_table[ch->class_num].default_group, FALSE);
				ch->pcdata->learned[gsn_language_arcane] = 100;	
				BIT_SET(ch->pcdata->traits, TRAIT_MAGAPT);
				send_to_char( "You choose to be reborn as a scholar of the earth/spirit spheres. So be it.\n\r", ch );
				reclassmsg(ch);
				return;
			}
		
			if ( !strcmp(arg3,"void"))
			{
				ch->class_num = 1;
				ch->pcdata->major_sphere = SPH_EARTH;
				ch->pcdata->minor_sphere = SPH_VOID;
				group_add(ch, "minor void v2", FALSE);
				group_add(ch, "mixed earth void", false);
				group_add(ch, class_table[ch->class_num].base_group, FALSE);
				group_add(ch, class_table[ch->class_num].default_group, FALSE);
				ch->pcdata->learned[gsn_language_arcane] = 100;	
				BIT_SET(ch->pcdata->traits, TRAIT_MAGAPT);
				send_to_char( "You choose to be reborn as a scholar of the earth/void spheres. So be it.\n\r", ch );
				reclassmsg(ch);
				return;
			}
		}
		
		if ( !strcmp(arg2,"fire"))
		{
			if ( !strcmp(arg3,"earth"))
			{
				ch->class_num = 5;
				ch->pcdata->major_sphere = SPH_FIRE;
				ch->pcdata->minor_sphere = SPH_EARTH;
				group_add(ch, "minor earth v2", FALSE);
				group_add(ch, "mixed fire earth", false);
				group_add(ch, class_table[ch->class_num].base_group, FALSE);
				group_add(ch, class_table[ch->class_num].default_group, FALSE);
				ch->pcdata->learned[gsn_language_arcane] = 100;
				BIT_SET(ch->pcdata->traits, TRAIT_MAGAPT);				
				send_to_char( "You choose to be reborn as a scholar of the fire/earth spheres. So be it.\n\r", ch );
				reclassmsg(ch);
				return;
			}
			
			if ( !strcmp(arg3,"fire"))
			{
				ch->class_num = 5;
				ch->pcdata->major_sphere = SPH_FIRE;
				ch->pcdata->minor_sphere = SPH_FIRE;
				group_add(ch, class_table[ch->class_num].base_group, FALSE);
				group_add(ch, class_table[ch->class_num].default_group, FALSE);
				ch->pcdata->learned[gsn_language_arcane] = 100;	
				BIT_SET(ch->pcdata->traits, TRAIT_MAGAPT);
				send_to_char( "You choose to be reborn as a greater scholar of the fire sphere. So be it.\n\r", ch );
				reclassmsg(ch);
				return;
			}
		
			if ( !strcmp(arg3,"water"))
			{
				ch->class_num = 5;
				ch->pcdata->major_sphere = SPH_FIRE;
				ch->pcdata->minor_sphere = SPH_WATER;
				group_add(ch, "minor water v2", FALSE);
				group_add(ch, "mixed fire water", false);
				group_add(ch, class_table[ch->class_num].base_group, FALSE);
				group_add(ch, class_table[ch->class_num].default_group, FALSE);
				ch->pcdata->learned[gsn_language_arcane] = 100;	
				BIT_SET(ch->pcdata->traits, TRAIT_MAGAPT);
				send_to_char( "You choose to be reborn as a scholar of the fire/water spheres. So be it.\n\r", ch );
				reclassmsg(ch);
				return;
			}
		
			if ( !strcmp(arg3,"air"))
			{
				ch->class_num = 5;
				ch->pcdata->major_sphere = SPH_FIRE;
				ch->pcdata->minor_sphere = SPH_AIR;
				group_add(ch, "minor air v2", FALSE);
				group_add(ch, "mixed fire air", false);
				group_add(ch, class_table[ch->class_num].base_group, FALSE);
				group_add(ch, class_table[ch->class_num].default_group, FALSE);
				ch->pcdata->learned[gsn_language_arcane] = 100;	
				BIT_SET(ch->pcdata->traits, TRAIT_MAGAPT);
				send_to_char( "You choose to be reborn as a scholar of the fire/air spheres. So be it.\n\r", ch );
				reclassmsg(ch);
				return;
			}
		
			if ( !strcmp(arg3,"spirit"))
			{
				ch->class_num = 5;
				ch->pcdata->major_sphere = SPH_FIRE;
				ch->pcdata->minor_sphere = SPH_SPIRIT;
				group_add(ch, "minor spirit v2", FALSE);
				group_add(ch, "mixed fire spirit", false);
				group_add(ch, class_table[ch->class_num].base_group, FALSE);
				group_add(ch, class_table[ch->class_num].default_group, FALSE);
				ch->pcdata->learned[gsn_language_arcane] = 100;	
				BIT_SET(ch->pcdata->traits, TRAIT_MAGAPT);
				send_to_char( "You choose to be reborn as a scholar of the fire/spirit spheres. So be it.\n\r", ch );
				reclassmsg(ch);
				return;
			}
		
			if ( !strcmp(arg3,"void"))
			{
				ch->class_num = 5;
				ch->pcdata->major_sphere = SPH_FIRE;
				ch->pcdata->minor_sphere = SPH_VOID;
				group_add(ch, "minor void v2", FALSE);
				group_add(ch, "mixed fire void", false);
				group_add(ch, class_table[ch->class_num].base_group, FALSE);
				group_add(ch, class_table[ch->class_num].default_group, FALSE);
				ch->pcdata->learned[gsn_language_arcane] = 100;	
				BIT_SET(ch->pcdata->traits, TRAIT_MAGAPT);
				send_to_char( "You choose to be reborn as a scholar of the fire/void spheres. So be it.\n\r", ch );
				reclassmsg(ch);
				return;
			}
		}
		
		if ( !strcmp(arg2,"water"))
		{
			if ( !strcmp(arg3,"earth"))
			{
				ch->class_num = 0;
				ch->pcdata->major_sphere = SPH_WATER;
				ch->pcdata->minor_sphere = SPH_EARTH;
				group_add(ch, "minor earth v2", FALSE);
				group_add(ch, "mixed water earth", false);
				group_add(ch, class_table[ch->class_num].base_group, FALSE);
				group_add(ch, class_table[ch->class_num].default_group, FALSE);
				ch->pcdata->learned[gsn_language_arcane] = 100;
				BIT_SET(ch->pcdata->traits, TRAIT_MAGAPT);
				send_to_char( "You choose to be reborn as a scholar of the water/earth spheres. So be it.\n\r", ch );
				reclassmsg(ch);
				return;
			}
			
			if ( !strcmp(arg3,"fire"))
			{
				ch->class_num = 0;
				ch->pcdata->major_sphere = SPH_WATER;
				ch->pcdata->minor_sphere = SPH_FIRE;
				group_add(ch, "minor fire v2", FALSE);
				group_add(ch, "mixed water fire", false);
				group_add(ch, class_table[ch->class_num].base_group, FALSE);
				group_add(ch, class_table[ch->class_num].default_group, FALSE);
				ch->pcdata->learned[gsn_language_arcane] = 100;
				BIT_SET(ch->pcdata->traits, TRAIT_MAGAPT);
				send_to_char( "You choose to be reborn as a scholar of the water/fire spheres. So be it.\n\r", ch );
				reclassmsg(ch);
				return;
			}
		
			if ( !strcmp(arg3,"water"))
			{
				ch->class_num = 0;
				ch->pcdata->major_sphere = SPH_WATER;
				ch->pcdata->minor_sphere = SPH_WATER;
				group_add(ch, class_table[ch->class_num].base_group, FALSE);
				group_add(ch, class_table[ch->class_num].default_group, FALSE);
				ch->pcdata->learned[gsn_language_arcane] = 100;
				BIT_SET(ch->pcdata->traits, TRAIT_MAGAPT);
				send_to_char( "You choose to be reborn as a greater scholar of the water sphere. So be it.\n\r", ch );
				reclassmsg(ch);
				return;
			}
		
			if ( !strcmp(arg3,"air"))
			{
				ch->class_num = 0;
				ch->pcdata->major_sphere = SPH_WATER;
				ch->pcdata->minor_sphere = SPH_AIR;
				group_add(ch, "minor air v2", FALSE);
				group_add(ch, "mixed water air", false);
				group_add(ch, class_table[ch->class_num].base_group, FALSE);
				group_add(ch, class_table[ch->class_num].default_group, FALSE);
				ch->pcdata->learned[gsn_language_arcane] = 100;
				BIT_SET(ch->pcdata->traits, TRAIT_MAGAPT);
				send_to_char( "You choose to be reborn as a scholar of the water/air spheres. So be it.\n\r", ch );
				reclassmsg(ch);
				return;
			}
		
			if ( !strcmp(arg3,"spirit"))
			{
				ch->class_num = 0;
				ch->pcdata->major_sphere = SPH_WATER;
				ch->pcdata->minor_sphere = SPH_SPIRIT;
				group_add(ch, "minor spirit v2", FALSE);
				group_add(ch, "mixed water spirit", false);
				group_add(ch, class_table[ch->class_num].base_group, FALSE);
				group_add(ch, class_table[ch->class_num].default_group, FALSE);
				ch->pcdata->learned[gsn_language_arcane] = 100;
				BIT_SET(ch->pcdata->traits, TRAIT_MAGAPT);
				send_to_char( "You choose to be reborn as a scholar of the water/spirit spheres. So be it.\n\r", ch );
				reclassmsg(ch);
				return;
			}
		
			if ( !strcmp(arg3,"void"))
			{
				ch->class_num = 0;
				ch->pcdata->major_sphere = SPH_WATER;
				ch->pcdata->minor_sphere = SPH_VOID;
				group_add(ch, "minor void v2", FALSE);
				group_add(ch, "mixed water void", false);
				group_add(ch, class_table[ch->class_num].base_group, FALSE);
				group_add(ch, class_table[ch->class_num].default_group, FALSE);
				ch->pcdata->learned[gsn_language_arcane] = 100;
				BIT_SET(ch->pcdata->traits, TRAIT_MAGAPT);
				send_to_char( "You choose to be reborn as a scholar of the water/void spheres. So be it.\n\r", ch );
				reclassmsg(ch);
				return;
			}
		}
		
		if ( !strcmp(arg2,"air"))
		{
			if ( !strcmp(arg3,"earth"))
			{
				ch->class_num = 4;
				ch->pcdata->major_sphere = SPH_AIR;
				ch->pcdata->minor_sphere = SPH_EARTH;
				group_add(ch, "minor earth v2", FALSE);
				group_add(ch, "mixed air earth", false);
				group_add(ch, class_table[ch->class_num].base_group, FALSE);
				group_add(ch, class_table[ch->class_num].default_group, FALSE);
				ch->pcdata->learned[gsn_language_arcane] = 100;
				BIT_SET(ch->pcdata->traits, TRAIT_MAGAPT);
				send_to_char( "You choose to be reborn as a scholar of the air/earth spheres. So be it.\n\r", ch );
				reclassmsg(ch);
				return;
			}
			
			if ( !strcmp(arg3,"fire"))
			{
				ch->class_num = 4;
				ch->pcdata->major_sphere = SPH_AIR;
				ch->pcdata->minor_sphere = SPH_FIRE;
				group_add(ch, "minor fire v2", FALSE);
				group_add(ch, "mixed air fire", false);
				group_add(ch, class_table[ch->class_num].base_group, FALSE);
				group_add(ch, class_table[ch->class_num].default_group, FALSE);
				ch->pcdata->learned[gsn_language_arcane] = 100;
				BIT_SET(ch->pcdata->traits, TRAIT_MAGAPT);
				send_to_char( "You choose to be reborn as a scholar of the air/fire spheres. So be it.\n\r", ch );
				reclassmsg(ch);
				return;
			}
		
			if ( !strcmp(arg3,"water"))
			{
				ch->class_num = 4;
				ch->pcdata->major_sphere = SPH_AIR;
				ch->pcdata->minor_sphere = SPH_WATER;
				group_add(ch, "minor water v2", FALSE);
				group_add(ch, "mixed air water", false);
				group_add(ch, class_table[ch->class_num].base_group, FALSE);
				group_add(ch, class_table[ch->class_num].default_group, FALSE);
				ch->pcdata->learned[gsn_language_arcane] = 100;
				BIT_SET(ch->pcdata->traits, TRAIT_MAGAPT);
				send_to_char( "You choose to be reborn as a scholar of the air/water spheres. So be it.\n\r", ch );
				reclassmsg(ch);
				return;
			}
		
			if ( !strcmp(arg3,"air"))
			{
				ch->class_num = 4;
				ch->pcdata->major_sphere = SPH_AIR;
				ch->pcdata->minor_sphere = SPH_AIR;
				group_add(ch, class_table[ch->class_num].base_group, FALSE);
				group_add(ch, class_table[ch->class_num].default_group, FALSE);
				ch->pcdata->learned[gsn_language_arcane] = 100;
				BIT_SET(ch->pcdata->traits, TRAIT_MAGAPT);
				send_to_char( "You choose to be reborn as a greater scholar of the air sphere. So be it.\n\r", ch );
				reclassmsg(ch);
				return;
			}
		
			if ( !strcmp(arg3,"spirit"))
			{
				ch->class_num = 4;
				ch->pcdata->major_sphere = SPH_AIR;
				ch->pcdata->minor_sphere = SPH_SPIRIT;
				group_add(ch, "minor spirit v2", FALSE);
				group_add(ch, "mixed air spirit", false);
				group_add(ch, class_table[ch->class_num].base_group, FALSE);
				group_add(ch, class_table[ch->class_num].default_group, FALSE);
				ch->pcdata->learned[gsn_language_arcane] = 100;
				BIT_SET(ch->pcdata->traits, TRAIT_MAGAPT);
				send_to_char( "You choose to be reborn as a scholar of the air/spirit spheres. So be it.\n\r", ch );
				reclassmsg(ch);
				return;
			}
		
			if ( !strcmp(arg3,"void"))
			{
				ch->class_num = 4;
				ch->pcdata->major_sphere = SPH_AIR;
				ch->pcdata->minor_sphere = SPH_VOID;
				group_add(ch, "minor void v2", FALSE);
				group_add(ch, "mixed air void", false);
				group_add(ch, class_table[ch->class_num].base_group, FALSE);
				group_add(ch, class_table[ch->class_num].default_group, FALSE);
				ch->pcdata->learned[gsn_language_arcane] = 100;
				BIT_SET(ch->pcdata->traits, TRAIT_MAGAPT);
				send_to_char( "You choose to be reborn as a scholar of the air/void spheres. So be it.\n\r", ch );
				reclassmsg(ch);
				return;
			}
		}
		
		if ( !strcmp(arg2,"spirit"))
		{
			if ( !strcmp(arg3,"earth"))
			{
				ch->class_num = 3;
				ch->pcdata->major_sphere = SPH_SPIRIT;
				ch->pcdata->minor_sphere = SPH_EARTH;
				group_add(ch, "minor earth v2", FALSE);
				group_add(ch, "mixed spirit earth", false);
				group_add(ch, class_table[ch->class_num].base_group, FALSE);
				group_add(ch, class_table[ch->class_num].default_group, FALSE);
				ch->pcdata->learned[gsn_language_arcane] = 100;
				BIT_SET(ch->pcdata->traits, TRAIT_MAGAPT);
				send_to_char( "You choose to be reborn as a scholar of the spirit/earth spheres. So be it.\n\r", ch );
				reclassmsg(ch);
				return;
			}
			
			if ( !strcmp(arg3,"fire"))
			{
				ch->class_num = 3;
				ch->pcdata->major_sphere = SPH_SPIRIT;
				ch->pcdata->minor_sphere = SPH_FIRE;
				group_add(ch, "minor fire v2", FALSE);
				group_add(ch, "mixed spirit fire", false);
				group_add(ch, class_table[ch->class_num].base_group, FALSE);
				group_add(ch, class_table[ch->class_num].default_group, FALSE);
				ch->pcdata->learned[gsn_language_arcane] = 100;
				BIT_SET(ch->pcdata->traits, TRAIT_MAGAPT);
				send_to_char( "You choose to be reborn as a scholar of the spirit/fire spheres. So be it.\n\r", ch );
				reclassmsg(ch);
				return;
			}
		
			if ( !strcmp(arg3,"water"))
			{
				ch->class_num = 3;
				ch->pcdata->major_sphere = SPH_SPIRIT;
				ch->pcdata->minor_sphere = SPH_WATER;
				group_add(ch, "minor water v2", FALSE);
				group_add(ch, "mixed spirit water", false);
				group_add(ch, class_table[ch->class_num].base_group, FALSE);
				group_add(ch, class_table[ch->class_num].default_group, FALSE);
				ch->pcdata->learned[gsn_language_arcane] = 100;
				BIT_SET(ch->pcdata->traits, TRAIT_MAGAPT);
				send_to_char( "You choose to be reborn as a scholar of the spirit/water spheres. So be it.\n\r", ch );
				reclassmsg(ch);
				return;
			}
		
			if ( !strcmp(arg3,"air"))
			{
				ch->class_num = 3;
				ch->pcdata->major_sphere = SPH_SPIRIT;
				ch->pcdata->minor_sphere = SPH_AIR;
				group_add(ch, "minor air v2", FALSE);
				group_add(ch, "mixed spirit air", false);
				group_add(ch, class_table[ch->class_num].base_group, FALSE);
				group_add(ch, class_table[ch->class_num].default_group, FALSE);
				ch->pcdata->learned[gsn_language_arcane] = 100;
				BIT_SET(ch->pcdata->traits, TRAIT_MAGAPT);
				send_to_char( "You choose to be reborn as a scholar of the spirit/air spheres. So be it.\n\r", ch );
				reclassmsg(ch);
				return;
			}
		
			if ( !strcmp(arg3,"spirit"))
			{
				ch->class_num = 3;
				ch->pcdata->major_sphere = SPH_SPIRIT;
				ch->pcdata->minor_sphere = SPH_SPIRIT;
				group_add(ch, class_table[ch->class_num].base_group, FALSE);
				group_add(ch, class_table[ch->class_num].default_group, FALSE);
				ch->pcdata->learned[gsn_language_arcane] = 100;
				BIT_SET(ch->pcdata->traits, TRAIT_MAGAPT);
				send_to_char( "You choose to be reborn as a greater scholar of the spirit sphere. So be it.\n\r", ch );
				reclassmsg(ch);
				return;
			}
		
			if ( !strcmp(arg3,"void"))
			{
				ch->class_num = 3;
				ch->pcdata->major_sphere = SPH_SPIRIT;
				ch->pcdata->minor_sphere = SPH_VOID;
				group_add(ch, "minor void v2", FALSE);
				group_add(ch, "mixed spirit void", false);
				group_add(ch, class_table[ch->class_num].base_group, FALSE);
				group_add(ch, class_table[ch->class_num].default_group, FALSE);
				ch->pcdata->learned[gsn_language_arcane] = 100;
				BIT_SET(ch->pcdata->traits, TRAIT_MAGAPT);
				send_to_char( "You choose to be reborn as a scholar of the scholar of the spirit/void spheres. So be it.\n\r", ch );
				reclassmsg(ch);
				return;
			}
		}
		
		if ( !strcmp(arg2,"void"))
		{
			if ( !strcmp(arg3,"earth"))
			{
				ch->class_num = 2;
				ch->pcdata->major_sphere = SPH_VOID;
				ch->pcdata->minor_sphere = SPH_EARTH;
				group_add(ch, "minor earth v2", FALSE);
				group_add(ch, "mixed void earth", false);
				group_add(ch, class_table[ch->class_num].base_group, FALSE);
				group_add(ch, class_table[ch->class_num].default_group, FALSE);
				ch->pcdata->learned[gsn_language_arcane] = 100;
				BIT_SET(ch->pcdata->traits, TRAIT_MAGAPT);
				BIT_SET(ch->pcdata->traits, TRAIT_OBSCURE);
				BIT_SET(ch->pcdata->traits, TRAIT_MORTICIAN);
				send_to_char( "You choose to be reborn as a scholar of the void/earth spheres. So be it.\n\r", ch );
				reclassmsg(ch);
				return;
			}
			
			if ( !strcmp(arg3,"fire"))
			{
				ch->class_num = 2;
				ch->pcdata->major_sphere = SPH_VOID;
				ch->pcdata->minor_sphere = SPH_FIRE;
				group_add(ch, "minor fire v2", FALSE);
				group_add(ch, "mixed void fire", false);
				group_add(ch, class_table[ch->class_num].base_group, FALSE);
				group_add(ch, class_table[ch->class_num].default_group, FALSE);
				ch->pcdata->learned[gsn_language_arcane] = 100;
				BIT_SET(ch->pcdata->traits, TRAIT_MAGAPT);
				BIT_SET(ch->pcdata->traits, TRAIT_OBSCURE);
				BIT_SET(ch->pcdata->traits, TRAIT_MORTICIAN);
				send_to_char( "You choose to be reborn as a scholar of the void/fire spheres. So be it.\n\r", ch );
				reclassmsg(ch);
				return;
			}
		
			if ( !strcmp(arg3,"water"))
			{
				ch->class_num = 2;
				ch->pcdata->major_sphere = SPH_VOID;
				ch->pcdata->minor_sphere = SPH_WATER;
				group_add(ch, "minor water v2", FALSE);
				group_add(ch, "mixed void water", false);
				group_add(ch, class_table[ch->class_num].base_group, FALSE);
				group_add(ch, class_table[ch->class_num].default_group, FALSE);
				ch->pcdata->learned[gsn_language_arcane] = 100;
				BIT_SET(ch->pcdata->traits, TRAIT_MAGAPT);
				BIT_SET(ch->pcdata->traits, TRAIT_OBSCURE);
				BIT_SET(ch->pcdata->traits, TRAIT_MORTICIAN);
				send_to_char( "You choose to be reborn as a scholar of the void/water spheres. So be it.\n\r", ch );
				reclassmsg(ch);
				return;
			}
		
			if ( !strcmp(arg3,"air"))
			{
				ch->class_num = 2;
				ch->pcdata->major_sphere = SPH_VOID;
				ch->pcdata->minor_sphere = SPH_AIR;
				group_add(ch, "minor air v2", FALSE);
				group_add(ch, "mixed void air", false);
				group_add(ch, class_table[ch->class_num].base_group, FALSE);
				group_add(ch, class_table[ch->class_num].default_group, FALSE);
				ch->pcdata->learned[gsn_language_arcane] = 100;
				BIT_SET(ch->pcdata->traits, TRAIT_MAGAPT);
				BIT_SET(ch->pcdata->traits, TRAIT_OBSCURE);
				BIT_SET(ch->pcdata->traits, TRAIT_MORTICIAN);
				send_to_char( "You choose to be reborn as a scholar of the void/air spheres. So be it.\n\r", ch );
				reclassmsg(ch);
				return;
			}
		
			if ( !strcmp(arg3,"spirit"))
			{
				ch->class_num = 2;
				ch->pcdata->major_sphere = SPH_VOID;
				ch->pcdata->minor_sphere = SPH_SPIRIT;
				group_add(ch, "minor spirit v2", FALSE);
				group_add(ch, "mixed void spirit", false);
				group_add(ch, class_table[ch->class_num].base_group, FALSE);
				group_add(ch, class_table[ch->class_num].default_group, FALSE);
				ch->pcdata->learned[gsn_language_arcane] = 100;
				BIT_SET(ch->pcdata->traits, TRAIT_MAGAPT);
				BIT_SET(ch->pcdata->traits, TRAIT_OBSCURE);
				BIT_SET(ch->pcdata->traits, TRAIT_MORTICIAN);
				send_to_char( "You choose to be reborn as a scholar of the void/spirit spheres. So be it.\n\r", ch );
				reclassmsg(ch);
				return;
			}
		
			if ( !strcmp(arg3,"void"))
			{
				ch->class_num = 2;
				ch->pcdata->major_sphere = SPH_VOID;
				ch->pcdata->minor_sphere = SPH_VOID;
				group_add(ch, class_table[ch->class_num].base_group, FALSE);
				group_add(ch, class_table[ch->class_num].default_group, FALSE);
				ch->pcdata->learned[gsn_language_arcane] = 100;
				BIT_SET(ch->pcdata->traits, TRAIT_MAGAPT);
				BIT_SET(ch->pcdata->traits, TRAIT_OBSCURE);
				BIT_SET(ch->pcdata->traits, TRAIT_MORTICIAN);
				send_to_char( "You choose to be reborn as a greater scholar of the void sphere. So be it.\n\r", ch );
				reclassmsg(ch);
				return;
			}
		}
	
	send_to_char( "You must choose the appropriates minor and major spheres for the scholar class. Avaliable choices are: earth, fire, water, air, void and spirit.\n\r", ch );
    return;
	}
	
    send_to_char( "Syntax: reclass <class name> <minor sphere> <major sphere>\n\r", ch );
    return;

}
	