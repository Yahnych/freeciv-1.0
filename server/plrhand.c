/********************************************************************** 
 Freeciv - Copyright (C) 1996 - A Kjeldberg, L Gregersen, P Unold
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
***********************************************************************/
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "player.h"
#include "unit.h"
#include "player.h"
#include "shared.h"
#include "packets.h"
#include "game.h"
#include "tech.h"
#include "plrhand.h"
#include "cityhand.h" 
#include "unithand.h" 
#include "map.h"
long lrand48(void);


extern struct advance advances[];
extern struct player_race races[];

void update_player_aliveness(struct player *pplayer);



struct player_score_entry {
  int idx;
  int value;
};

int secompare(const void *a, const void *b)
{
  
  return (((struct player_score_entry *)b)->value-((struct player_score_entry *)a)->value);
}

char *greatness[]={"Magnificent", "Glorious", "Great", "Decent", "Mediocre", "Hilarious", "Worthless", "Pathetic", "Useless","Useless","Useless","Useless","Useless","Useless"};

void historian_richest()
{
  int i;
  char buffer[4096];
  char buf2[4096];
  struct player_score_entry *size=(struct player_score_entry *)malloc(sizeof(struct player_score_entry)*game.nplayers);

  for (i=0;i<game.nplayers;i++) {
    size[i].value=game.players[i].economic.gold;
    size[i].idx=i;
  }
  qsort(size, game.nplayers, sizeof(struct player_score_entry), secompare);
  buffer[0]=0;
  for (i=0;i<game.nplayers;i++) {
    sprintf(buf2,"%2d: The %s %s\n",i+1, greatness[i],  
	    get_race_name_plural(game.players[size[i].idx].race));
    strcat(buffer,buf2);
  }
  free(size);
  page_player(0, "Herodot's report on the RICHEST Civilizations in the World.", buffer);
}

void historian_advanced()
{
  int i;
  char buffer[4096];
  char buf2[4096];
  struct player_score_entry *size=(struct player_score_entry *)malloc(sizeof(struct player_score_entry)*game.nplayers);

  for (i=0;i<game.nplayers;i++) {
    size[i].value=game.players[i].score.techs;
    size[i].idx=i;
  }
  qsort(size, game.nplayers, sizeof(struct player_score_entry), secompare);
  buffer[0]=0;
  for (i=0;i<game.nplayers;i++) {
    sprintf(buf2,"%2d: The %s %s\n",i+1, greatness[i], get_race_name_plural(game.players[size[i].idx].race));
    strcat(buffer,buf2);
  }
  free(size);
  page_player(0, "Herodot's report on the most ADVANCED Civilizations in the World.", buffer);
  
}

void historian_military()
{
  int i;
  char buffer[4096];
  char buf2[4096];
  struct player_score_entry *size=(struct player_score_entry *)malloc(sizeof(struct player_score_entry)*game.nplayers);

  for (i=0;i<game.nplayers;i++) {
    size[i].value=game.players[i].score.units;
    size[i].idx=i;
  }
  qsort(size, game.nplayers, sizeof(struct player_score_entry), secompare);
  buffer[0]=0;
  for (i=0;i<game.nplayers;i++) {
    sprintf(buf2,"%2d: The %s %s\n",i+1, greatness[i], get_race_name_plural(game.players[size[i].idx].race));
    strcat(buffer,buf2);
  }
  free(size);
  page_player(0, "Herodots report on the most MILITARIZED Civilizations in the World.", buffer);
  
}

void historian_happiest()
{
  int i;
  char buffer[4096];
  char buf2[4096];
  struct player_score_entry *size=(struct player_score_entry *)malloc(sizeof(struct player_score_entry)*game.nplayers);

  for (i=0;i<game.nplayers;i++) {
    size[i].value=
      ((game.players[i].score.happy-game.players[i].score.unhappy)*1000)
      /(1+total_player_citizens(&game.players[i]));
    size[i].idx=i;
  }
  qsort(size, game.nplayers, sizeof(struct player_score_entry), secompare);
  buffer[0]=0;
  for (i=0;i<game.nplayers;i++) {
    sprintf(buf2,"%2d: The %s %s\n",i+1, greatness[i], get_race_name_plural(game.players[size[i].idx].race));
    strcat(buffer,buf2);
  }
  free(size);
  page_player(0, "Herodot's report on the HAPPIEST Civilizations in the World.", buffer);
}  

void historian_largest()
{
  int i;
  char buffer[4096];
  char buf2[4096];
  struct player_score_entry *size=(struct player_score_entry *)malloc(sizeof(struct player_score_entry)*game.nplayers);

  for (i=0;i<game.nplayers;i++) {
    size[i].value=total_player_citizens(&game.players[i]);
    size[i].idx=i;
  }
  qsort(size, game.nplayers, sizeof(struct player_score_entry), secompare);
  buffer[0]=0;
  for (i=0;i<game.nplayers;i++) {
    sprintf(buf2,"%2d: The %s %s\n",i+1, greatness[i], get_race_name_plural(game.players[size[i].idx].race));
    strcat(buffer,buf2);
  }
  free(size);
  page_player(0, "Herodot's report on the LARGEST Civilizations in the World.", buffer);
}

int nr_wonders(struct city *pcity)
{
  int i;
  int res=0;
  for (i=0;i<B_LAST;i++)
    if (is_wonder(i) && city_got_building(pcity, i))
      res++;
  return res;
}

void top_five_cities(struct player *pplayer)
{
  struct player_score_entry *size=(struct player_score_entry *)malloc(sizeof(struct player_score_entry)*5);
  struct city *pcity;
    int i;
  char buffer[4096];
  char buf2[4096];
  buffer[0]=0;
  for (i=0;i<5;i++) {
    size[i].value=0;
    size[i].idx=0;
  }
  for (i=0;i<game.nplayers;i++) {
    struct genlist_iterator myiter;
    genlist_iterator_init(&myiter, &game.players[i].cities.list, 0);
    for(; ITERATOR_PTR(myiter); ITERATOR_NEXT(myiter)) {
      pcity=(struct city *)ITERATOR_PTR(myiter);
      if ((pcity->size+nr_wonders(pcity)*5)>size[4].value) {
	size[4].value=pcity->size+nr_wonders(pcity)*5;
	size[4].idx=pcity->id;
	qsort(size, 5, sizeof(struct player_score_entry), secompare);
      }
    }
  }
  for (i=0;i<5;i++) {
    pcity=find_city_by_id(size[i].idx);
    if (pcity) { 
      sprintf(buf2, "%2d: The %s City of %s of size %d, with %d wonders\n", i+1,  
	      get_race_name(city_owner(pcity)->race),pcity->name, 
	      pcity->size, nr_wonders(pcity));
      strcat(buffer, buf2);
    }
  }
  free(size);
  page_player(pplayer, "The 5 greatest cities in the world!", buffer);
}

void wonders_of_the_world(struct player *pplayer)
{
  int i;
  struct city *pcity;
  char buffer[4096];
  char buf2[4096];
  buffer[0]=0;
  for (i=0;i<B_LAST;i++) {
    if(is_wonder(i) && game.global_wonders[i] && 
       (pcity=find_city_by_id(game.global_wonders[i]))) {
      sprintf(buf2, "%s (%s) have the %s\n", pcity->name, 
	      get_race_name(game.players[pcity->owner].race), 
	      get_improvement_name(i));
      strcat(buffer, buf2);
    }
  }
  page_player(pplayer, "Wonders Of the World.", buffer);
}
int rank_calc_research(struct player *pplayer)
{
  return (pplayer->score.techout*100)/(1+research_time(pplayer));
}

int rank_research(struct player *pplayer)
{
  int basis=rank_calc_research(pplayer);
  int place=1;
  int i;
  for (i=0;i<game.nplayers;i++) {
    if (rank_calc_research(&game.players[i])>basis)
      place++;
  }
  return place;
}

int rank_calc_literacy(struct player *pplayer)
{
  return (pplayer->score.literacy*100)/(1+civ_population(pplayer));
}

int rank_literacy(struct player *pplayer)
{
  int basis=rank_calc_literacy(pplayer);
  int place=1;
  int i;
  for (i=0;i<game.nplayers;i++) {
    if (rank_calc_literacy(&game.players[i])>basis)
      place++;
  }
  return place;
}

int rank_production(struct player *pplayer)
{
  int basis=pplayer->score.mfg;
  int place=1;
  int i;
  for (i=0;i<game.nplayers;i++) {
    if (game.players[i].score.mfg>basis)
      place++;
  }
  return place;
}

int rank_economics(struct player *pplayer)
{
  int basis=pplayer->score.bnp;
  int place=1;
  int i;
  for (i=0;i<game.nplayers;i++) {
    if (game.players[i].score.bnp>basis)
      place++;
  }
  return place;
}

int rank_polution(struct player *pplayer)
{
  int basis=pplayer->score.polution;
  int place=1;
  int i;
  for (i=0;i<game.nplayers;i++) {
    if (game.players[i].score.polution<basis)
      place++;
  }
  return place;
}

int rank_calc_mil_service(struct player *pplayer)
{
 return (pplayer->score.units*50000)/(100+civ_population(pplayer)/100);
}

int rank_mil_service(struct player *pplayer)
{
  int basis=rank_calc_mil_service(pplayer);
  int place=1;
  int i;
  for (i=0;i<game.nplayers;i++) {
    if (rank_calc_mil_service(&game.players[i])<basis)
      place++;
  }
  return place;
}

char *number_to_string(int x)
{
  static char buf[4];
  buf[3]=0;
  if (x<0 || x>99) x=0;
  sprintf(buf, "%dth",x);
  if (x==1) { buf[1]='s'; buf[2]='t';}
  if (x==2) { buf[1]='n'; buf[2]='d';}
  if (x==3) { buf[1]='r'; buf[2]='d';}
  return buf;
}

void demographics_report(struct player *pplayer)
{
  char buffer[4096];
  char buf2[4096];
  buffer[0]=0;

  sprintf(buf2, "%-20s:%d%% (%s)\n", "Research Speed", rank_calc_research(pplayer), number_to_string(rank_research(pplayer)));
  strcat(buffer, buf2);

  sprintf(buf2, "%-20s:%d%% (%s)\n", "Literacy",rank_calc_literacy(pplayer), number_to_string(rank_literacy(pplayer)));
  strcat(buffer, buf2);
  sprintf(buf2, "%-20s:%d M. MFG (%s)\n", "Production",  pplayer->score.mfg, number_to_string(rank_production(pplayer)));
  strcat(buffer, buf2);
  sprintf(buf2, "%-20s:%d M. BNP (%s)\n", "Economics", pplayer->score.bnp, number_to_string(rank_economics(pplayer)));
  strcat(buffer, buf2);
  
  sprintf(buf2, "%-20s:%d Months (%s)\n", "Military service", rank_calc_mil_service(pplayer), number_to_string(rank_mil_service(pplayer)));
  strcat(buffer, buf2);

  sprintf(buf2, "%-20s:%d Tons (%s)\n", "Polution", pplayer->score.polution, number_to_string(rank_polution(pplayer)));
  strcat(buffer, buf2);
  
  page_player(pplayer, "Demographics report.       ", buffer);
}

void make_history_report()
{
  static int report=0;
  static int time_to_report=20;
  int i;
  if (game.nplayers==1)
    return;
  for (i=0;i<game.nplayers;i++) 
    civ_score(&game.players[i]);
  time_to_report--;
  
  if (time_to_report>0) 
    return;

  time_to_report=(lrand48()%20)+20;
  
  switch (report) {
  case 0:
    historian_richest();
    break;
  case 1:
    historian_advanced();
    break;
  case 2:
    historian_largest();
    break;
  case 3:
    historian_happiest();
    break;
  case 4:
    historian_military();
    break;
  }
  report=(report+1)%5;
}

/**************************************************************************
...
**************************************************************************/

void show_ending()
{
  int i;
  char buffer[4096];
  char buf2[4096];

  struct player_score_entry *size=(struct player_score_entry *)malloc(sizeof(struct player_score_entry)*game.nplayers);

  for (i=0;i<game.nplayers;i++) {
    size[i].value=civ_score(&game.players[i]);
    size[i].idx=i;
  }
  qsort(size, game.nplayers, sizeof(struct player_score_entry), secompare);
  buffer[0]=0;
  for (i=0;i<game.nplayers;i++) {
    sprintf(buf2,"%2d: The %s %s scored %d points\n",i+1, greatness[i],  
            get_race_name_plural(game.players[size[i].idx].race), size[i].value);
    strcat(buffer,buf2);
  }
  free(size);
  page_player(0, "The Greatest Civilizations in the world.", buffer);

}


/**************************************************************************
...
**************************************************************************/

void great_library(struct player *pplayer)
{
  int i;
  if (wonder_is_obsolete(B_GREAT)) 
    return;
  if (find_city_wonder(B_GREAT)) {
    if (pplayer->player_no==find_city_wonder(B_GREAT)->owner) {
      for (i=0;i<A_LAST;i++) {
	if (get_invention(pplayer, i)!=TECH_KNOWN 
	    && game.global_advances[i]>=2) {
	  notify_player(pplayer, "Game: %s acquired from The Great Library!", advances[i].name);
	  set_invention(pplayer, i, TECH_KNOWN);
	  remove_obsolete_buildings(pplayer);
	  pplayer->research.researchpoints++;
	  break;
	}
      }
    }
    if (get_invention(pplayer, pplayer->research.researching)==TECH_KNOWN) {
      choose_random_tech(pplayer);
      notify_player(pplayer, "Game: Our scientist now focuses on %s",
		    advances[pplayer->research.researching].name);
    }
  }
}

/**************************************************************************
Count down if the player are in a revolution, notify him when revolution
has ended.
**************************************************************************/

void update_revolution(struct player *pplayer)
{
  if(pplayer->revolution) 
    pplayer->revolution--;
}

/**************************************************************************
Main update loop, for each player at end of turn.
**************************************************************************/

void update_player_activities(struct player *pplayer) 
{
  notify_player(pplayer, "Year: %s", textyear(game.year)); 
  great_library(pplayer);
  update_revolution(pplayer);
  player_restore_units(pplayer);
  if (city_list_size(&pplayer->cities)) 
    update_tech(pplayer, city_list_size(&pplayer->cities));
  update_city_activities(pplayer);
  update_unit_activities(pplayer);
  update_player_aliveness(pplayer);
}

/**************************************************************************
...
**************************************************************************/

void update_player_aliveness(struct player *pplayer)
{
  if(pplayer->is_alive) {
    if(unit_list_size(&pplayer->units)==0 && 
       city_list_size(&pplayer->cities)==0) {
      pplayer->is_alive=0;
      notify_player(0, "Game: The %s are no more!", 
		    races[pplayer->race].name);
    }
  }
}

/**************************************************************************
Called from each city to update the research.
**************************************************************************/

void update_tech(struct player *plr, int bulbs)
{
  int old, new;
  int philohack=0;
  plr->research.researched+=bulbs;
  if (plr->research.researched < research_time(plr)) 
    return;
  plr->got_tech=1;
  plr->research.researchpoints++;
  old=plr->research.researching;
  if (old==A_PHILOSOPHY && !game.global_advances[A_PHILOSOPHY]) 
    philohack=1;
  set_invention(plr, plr->research.researching, TECH_KNOWN);
  remove_obsolete_buildings(plr);
  
  /* start select_tech dialog */ 
  choose_random_tech(plr);
  new=plr->research.researching;

  notify_player(plr, "Game: Learned %s. Researching %s.",advances[old].name,advances[new].name);
  
  if (philohack) {
    notify_player(plr, "Game: Great philosophers from all the world joins your civilization, you get an immediate advance");
    update_tech(plr, 1000000);
  }
}

/**************************************************************************
...
**************************************************************************/

void choose_random_tech(struct player *plr)
{
  int researchable=0;
  int i;
  int choosen;
  plr->research.researched=0;
  update_research(plr);
  for (i=0;i<A_LAST;i++)
    if (get_invention(plr, i)==TECH_REACHABLE) 
      researchable++;
  if (researchable==0) { 
    plr->research.researching=A_NONE;
    return;
  }
  choosen=(lrand48()%researchable)+1;
  
  for (i=0;i<A_LAST;i++)
    if (get_invention(plr, i)==TECH_REACHABLE) {
      choosen--;
      if (!choosen) break;
    }
  plr->research.researching=i;
}

/**************************************************************************
...
**************************************************************************/

void choose_tech(struct player *plr, int tech)
{
  if (plr->research.researching==tech)
    return;
  update_research(plr);
  if (get_invention(plr, tech)!=TECH_REACHABLE) { /* can't research this */
    return;
  }
  plr->research.researching=tech;
  if (!plr->got_tech)
    plr->research.researched=0;     /* Reset tech because we changed subject */
}

/**************************************************************************
...
**************************************************************************/

void init_tech(struct player *plr, int tech)
{
  int i;
  for (i=0;i<A_LAST;i++) 
    set_invention(plr, i, 0);
  set_invention(plr, A_NONE, TECH_KNOWN);
  
  plr->research.researchpoints=1;
  for (i=0;i<tech;i++) {
    choose_random_tech(plr);
    set_invention(plr, plr->research.researching, TECH_KNOWN);
  }
  choose_random_tech(plr);
}

/**************************************************************************
...
**************************************************************************/

void handle_player_rates(struct player *pplayer, 
                         struct packet_player_request *preq)
{
  if (preq->tax+preq->luxury+preq->science!=100)
    return;
  if (preq->tax<0 || preq->tax >100) return;
  if (preq->luxury<0 || preq->luxury > 100) return;
  if (preq->science<0 || preq->science >100) return;
  pplayer->economic.tax=preq->tax;
  pplayer->economic.luxury=preq->luxury;
  pplayer->economic.science=preq->science;
  send_player_info(pplayer, pplayer); 
}

/**************************************************************************
...
**************************************************************************/

void handle_player_research(struct player *pplayer,
			    struct packet_player_request *preq)
{
  choose_tech(pplayer, preq->tech);
  send_player_info(pplayer, pplayer);
}


/**************************************************************************
...
**************************************************************************/
void handle_player_goverment(struct player *pplayer,
			     struct packet_player_request *preq)
{
  if (pplayer->revolution || pplayer->goverment!=G_ANARCHY 
      || !can_change_to_goverment(pplayer, preq->goverment))
    return;
  pplayer->goverment=preq->goverment;
  notify_player(pplayer, "Game: %s now governs the %s as a %s.", 
		pplayer->name, 
		races[pplayer->race].name, 
		get_goverment_name(preq->goverment));  
  send_player_info(pplayer, pplayer);
}

/**************************************************************************
...
**************************************************************************/

void handle_player_revolution(struct player *pplayer)
{
  struct city *pcity;
  if (pplayer->revolution || pplayer->goverment==G_ANARCHY) 
    return;
  pplayer->revolution=lrand48()%5+1;
  pplayer->goverment=G_ANARCHY;
  notify_player(pplayer, "Game: The %s have incited a revolt!", 
		races[pplayer->race].name);
  send_player_info(pplayer, pplayer);
  pcity=find_city_by_id(game.global_wonders[B_PYRAMIDS]);
  if (pcity && player_owns_city(pplayer,pcity)) {
    pplayer->revolution=1;
    return;
  }
}

/**************************************************************************
...
**************************************************************************/
void notify_player(struct player *pplayer, char *format, ...) 
{
  int i;
  struct packet_generic_message genmsg;
  va_list args;
  va_start(args, format);
  vsprintf(genmsg.message, format, args);
  va_end(args);
  
  for(i=0; i<game.nplayers; i++)
    if(!pplayer || pplayer==&game.players[i])
      send_packet_generic_message(game.players[i].conn, PACKET_CHAT_MSG, &genmsg);
}

/**************************************************************************
This function popup a none-modal message dialog on the player's desktop
**************************************************************************/
void page_player(struct player *pplayer, char *headline, char *lines) 
{
  int i;
  struct packet_generic_message genmsg;
  
  strcpy(genmsg.message, headline);
  strcat(genmsg.message, "\n");
  strcat(genmsg.message, lines);
  
  for(i=0; i<game.nplayers; i++)
    if(!pplayer || pplayer==&game.players[i])
      send_packet_generic_message(game.players[i].conn, PACKET_PAGE_MSG, 
				  &genmsg);
}

/**************************************************************************
both src and dest can be NULL
NULL means all players
**************************************************************************/
void send_player_info(struct player *src, struct player *dest)
{
  int o, i, j;

  for(o=0; o<game.nplayers; o++)           /* dests */
     if(!dest || &game.players[o]==dest)
        for(i=0; i<game.nplayers; i++)     /* srcs  */
           if(!src || &game.players[i]==src) {
             struct packet_player_info info;
             info.playerno=i;
             strcpy(info.name, game.players[i].name);
             info.race=game.players[i].race;

             info.gold=game.players[i].economic.gold;
             info.tax=game.players[i].economic.tax;
             info.science=game.players[i].economic.science;
             info.luxury=game.players[i].economic.luxury;
	     info.goverment=game.players[i].goverment;             
	     info.embassy=game.players[i].embassy;             
             
	     info.researched=game.players[i].research.researched;
             info.researchpoints=game.players[i].research.researchpoints;
             info.researching=game.players[i].research.researching;
             for(j=0; j<A_LAST; j++)
               info.inventions[j]=game.players[i].research.inventions[j]+'0';
             info.inventions[j]='\0';
             info.turn_done=game.players[i].turn_done;
             info.nturns_idle=game.players[i].nturns_idle;
             info.is_alive=game.players[i].is_alive;
             info.is_connected=game.players[i].is_connected;
             strcpy(info.addr, game.players[i].addr);
	     info.revolution=game.players[i].revolution;
             send_packet_player_info(game.players[o].conn, &info);
	   }
}

/***************************************************************
...
***************************************************************/
void player_load(struct player *plr, int plrno, struct section_file *file)
{
  int i, j, x, y, nunits, ncities;
  char *p;

  strcpy(plr->name, secfile_lookup_str(file, "player%d.name", plrno));
  plr->race=secfile_lookup_int(file, "player%d.race", plrno);
  plr->goverment=secfile_lookup_int(file, "player%d.goverment", plrno);
  plr->embassy=secfile_lookup_int(file, "player%d.embassy", plrno);
   
  strcpy(plr->addr, "---.---.---.---");

  plr->nturns_idle=0;
  plr->is_alive=secfile_lookup_int(file, "player%d.is_alive", plrno);

  plr->economic.gold=secfile_lookup_int(file, "player%d.gold", plrno);
  plr->economic.tax=secfile_lookup_int(file, "player%d.tax", plrno);
  plr->economic.science=secfile_lookup_int(file, "player%d.science", plrno);
  plr->economic.luxury=secfile_lookup_int(file, "player%d.luxury", plrno);

  plr->research.researched=secfile_lookup_int(file, 
					     "player%d.researched", plrno);
  plr->research.researchpoints=secfile_lookup_int(file, 
					     "player%d.researchpoints", plrno);
  plr->research.researching=secfile_lookup_int(file, 
					     "player%d.researching", plrno);

  p=secfile_lookup_str(file, "player%d.invs", plrno);
    
  plr->capital=secfile_lookup_int(file, "player%d.capital", plrno);

  for(i=0; i<A_LAST; i++)
    set_invention(plr, i, (p[i]=='1') ? TECH_KNOWN : TECH_UNKNOWN);
  
  update_research(plr);
    
  city_list_init(&plr->cities);
  ncities=secfile_lookup_int(file, "player%d.ncities", plrno);
  
  /* this should def. be placed in city.c later - PU */
  for(i=0; i<ncities; i++) { /* read the cities */
    struct city *pcity;
    
    pcity=(struct city *)malloc(sizeof(struct city));
    pcity->id=secfile_lookup_int(file, "player%d.c%d.id", plrno, i);
    pcity->owner=plrno;
    pcity->x=secfile_lookup_int(file, "player%d.c%d.x", plrno, i);
    pcity->y=secfile_lookup_int(file, "player%d.c%d.y", plrno, i);
    strcpy(pcity->name, secfile_lookup_str(file, "player%d.c%d.name",
					   plrno, i));
    pcity->size=secfile_lookup_int(file, "player%d.c%d.size", plrno, i);
    pcity->ppl_elvis=secfile_lookup_int(file, "player%d.c%d.nelvis", plrno, i);
    pcity->ppl_scientist=secfile_lookup_int(file, 
					  "player%d.c%d.nscientist", plrno, i);
    pcity->ppl_taxman=secfile_lookup_int(file, "player%d.c%d.ntaxman",
					 plrno, i);

    for(j=0; j<4; j++)
      pcity->trade[j]=secfile_lookup_int(file, "player%d.c%d.traderoute%d",
					 plrno, i, j);
    
    pcity->food_stock=secfile_lookup_int(file, "player%d.c%d.food_stock", 
						 plrno, i);
    pcity->shield_stock=secfile_lookup_int(file, "player%d.c%d.shield_stock", 
						   plrno, i);
    pcity->trade_prod=0;
    pcity->anarchy=secfile_lookup_int(file, "player%d.c%d.anarchy", plrno,i);
    pcity->was_happy=secfile_lookup_int(file, "player%d.c%d.was_happy", plrno,i);
    pcity->is_building_unit=secfile_lookup_int(file, 
				    "player%d.c%d.is_building_unit", plrno, i);

    pcity->currently_building=secfile_lookup_int(file, 
						 "player%d.c%d.currently_building", plrno, i);
    pcity->did_buy=secfile_lookup_int(file,
				      "player%d.c%d.did_buy", plrno,i);
   city_incite_cost(pcity);
    p=secfile_lookup_str(file, "player%d.c%d.workers", plrno, i);
    for(y=0; y<CITY_MAP_SIZE; y++)
      for(x=0; x<CITY_MAP_SIZE; x++)
	pcity->city_map[x][y]=(*p++=='1') ? C_TILE_WORKER : C_TILE_EMPTY;

    p=secfile_lookup_str(file, "player%d.c%d.improvements", plrno, i);
    
    for(x=0; x<B_LAST; x++)
      pcity->improvements[x]=(*p++=='1') ? 1 : 0;
    
    unit_list_init(&pcity->units_supported);

    map_get_tile(pcity->x, pcity->y)->city_id=pcity->id;
    
    city_list_insert(&plr->cities, pcity);
  }
  unit_list_init(&plr->units);
  nunits=secfile_lookup_int(file, "player%d.nunits", plrno);
  
  /* this should def. be placed in unit.c later - PU */
  for(i=0; i<nunits; i++) { /* read the units */
    struct unit *punit;
    struct city *pcity;
    
    punit=(struct unit *)malloc(sizeof(struct unit));
    punit->id=secfile_lookup_int(file, "player%d.u%d.id", plrno, i);
    punit->owner=plrno;
    punit->x=secfile_lookup_int(file, "player%d.u%d.x", plrno, i);
    punit->y=secfile_lookup_int(file, "player%d.u%d.y", plrno, i);

    punit->veteran=secfile_lookup_int(file, "player%d.u%d.veteran", plrno, i);
    punit->homecity=secfile_lookup_int(file, "player%d.u%d.homecity",
				       plrno, i);

    if((pcity=find_city_by_id(punit->homecity)))
      unit_list_insert(&pcity->units_supported, punit);
    else
      punit->homecity=0;
    
    punit->type=secfile_lookup_int(file, "player%d.u%d.type", plrno, i);

    punit->moves_left=get_unit_type(punit->type)->move_rate;
    punit->attacks_left=get_unit_type(punit->type)->no_attacks;
    punit->fuel=get_unit_type(punit->type)->fuel;
    punit->activity=secfile_lookup_int(file, "player%d.u%d.activity",plrno, i);
    punit->activity_count=secfile_lookup_int(file, 
					     "player%d.u%d.activity_count",
					     plrno, i);
    punit->hp=secfile_lookup_int(file, "player%d.u%d.hp", plrno, i);
    punit->bribe_cost=unit_bribe_cost(punit);
    unit_list_insert(&plr->units, punit);

    unit_list_insert(&map_get_tile(punit->x, punit->y)->units, punit);
  }

}

/***************************************************************
...
***************************************************************/
void player_save(struct player *plr, int plrno, struct section_file *file)
{
  int i;
  char invs[A_LAST+1];
  struct genlist_iterator myiter;

  secfile_insert_str(file, plr->name, "player%d.name", plrno);
  secfile_insert_int(file, plr->race, "player%d.race", plrno);
  secfile_insert_int(file, plr->goverment, "player%d.goverment", plrno);
  secfile_insert_int(file, plr->embassy, "player%d.embassy", plrno);
   
  secfile_insert_int(file, plr->is_alive, "player%d.is_alive", plrno);
  secfile_insert_int(file, plr->economic.gold, "player%d.gold", plrno);
  secfile_insert_int(file, plr->economic.tax, "player%d.tax", plrno);
  secfile_insert_int(file, plr->economic.science, "player%d.science", plrno);
  secfile_insert_int(file, plr->economic.luxury, "player%d.luxury", plrno);

  secfile_insert_int(file, plr->research.researched, 
		     "player%d.researched", plrno);
  secfile_insert_int(file, plr->research.researchpoints,
		     "player%d.researchpoints", plrno);
  secfile_insert_int(file, plr->research.researching,
		     "player%d.researching", plrno);  

  secfile_insert_int(file, plr->capital, 
		      "player%d.capital", plrno);

  for(i=0; i<A_LAST; i++)
    invs[i]=(get_invention(plr, i)==TECH_KNOWN) ? '1' : '0';
  invs[i]='\0';
  secfile_insert_str(file, invs, "player%d.invs", plrno);
  
  secfile_insert_int(file, unit_list_size(&plr->units), "player%d.nunits", 
		     plrno);
  secfile_insert_int(file, city_list_size(&plr->cities), "player%d.ncities", 
		     plrno);
  
  
  genlist_iterator_init(&myiter, &plr->units.list, 0);

  for(i=0; ITERATOR_PTR(myiter); ITERATOR_NEXT(myiter), i++) {
    struct unit *punit=(struct unit *)ITERATOR_PTR(myiter);

    secfile_insert_int(file, punit->id, "player%d.u%d.id", plrno, i);
    secfile_insert_int(file, punit->x, "player%d.u%d.x", plrno, i);
    secfile_insert_int(file, punit->y, "player%d.u%d.y", plrno, i);
    secfile_insert_int(file, punit->veteran, "player%d.u%d.veteran", 
				plrno, i);
    secfile_insert_int(file, punit->hp, "player%d.u%d.hp", plrno, i);
    secfile_insert_int(file, punit->homecity, "player%d.u%d.homecity",
				plrno, i);
    secfile_insert_int(file, punit->type, "player%d.u%d.type",
				plrno, i);
    secfile_insert_int(file, punit->activity, "player%d.u%d.activity",
				plrno, i);
    secfile_insert_int(file, punit->activity_count, 
				"player%d.u%d.activity_count",
				plrno, i);
  }

  genlist_iterator_init(&myiter, &plr->cities.list, 0);
  
  for(i=0; ITERATOR_PTR(myiter); ITERATOR_NEXT(myiter), i++) {
    int j, x, y;
    char buf[512];
    struct city *pcity=(struct city *)ITERATOR_PTR(myiter);

    
    secfile_insert_int(file, pcity->id, "player%d.c%d.id", plrno, i);
    secfile_insert_int(file, pcity->x, "player%d.c%d.x", plrno, i);
    secfile_insert_int(file, pcity->y, "player%d.c%d.y", plrno, i);
    secfile_insert_str(file, pcity->name, "player%d.c%d.name", plrno, i);
    secfile_insert_int(file, pcity->size, "player%d.c%d.size", plrno, i);

    secfile_insert_int(file, pcity->ppl_elvis, "player%d.c%d.nelvis", plrno, i);
    secfile_insert_int(file, pcity->ppl_scientist, "player%d.c%d.nscientist", 
		       plrno, i);
    secfile_insert_int(file, pcity->ppl_taxman, "player%d.c%d.ntaxman", plrno, i);

    for(j=0; j<4; j++)
      secfile_insert_int(file, pcity->trade[j], "player%d.c%d.traderoute%d", 
			 plrno, i, j);
    
    secfile_insert_int(file, pcity->food_stock, "player%d.c%d.food_stock", 
		       plrno, i);
    secfile_insert_int(file, pcity->shield_stock, "player%d.c%d.shield_stock", 
		       plrno, i);
    secfile_insert_int(file, pcity->anarchy, "player%d.c%d.anarchy", plrno,i);
    secfile_insert_int(file, pcity->was_happy, "player%d.c%d.was_happy", plrno,i);
    secfile_insert_int(file, pcity->did_buy, "player%d.c%d.did_buy", plrno,i);
    j=0;
    for(y=0; y<CITY_MAP_SIZE; y++)
      for(x=0; x<CITY_MAP_SIZE; x++)
	buf[j++]=(get_worker_city(pcity, x, y)==C_TILE_WORKER) ? '1' : '0';
    buf[j]='\0';
    secfile_insert_str(file, buf, "player%d.c%d.workers", plrno, i);
  

    secfile_insert_int(file, pcity->is_building_unit, 
		       "player%d.c%d.is_building_unit", plrno, i);
    secfile_insert_int(file, pcity->currently_building, 
		       "player%d.c%d.currently_building", plrno, i);

    for(j=0; j<B_LAST; j++)
      buf[j]=(pcity->improvements[j]) ? '1' : '0';
    buf[j]='\0';
    secfile_insert_str(file, buf, "player%d.c%d.improvements", plrno, i);
  }
}
