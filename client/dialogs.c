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
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/Toggle.h>     
#include <X11/Xaw/Command.h>
#include <X11/Xaw/AsciiText.h>  
#include <X11/Xaw/Dialog.h>

#include "dialogs.h"
#include "player.h"
#include "packets.h"
#include "chatline.h"
#include "map.h"
#include "mapview.h"
#include "game.h"
#include "mapctrl.h"
#include "xstuff.h"
#include "civclient.h"

extern Widget toplevel, main_form, map_canvas;

extern struct connection aconnection;
extern Display	*display;
extern int display_depth;

/******************************************************************/
Widget races_dialog_shell;
Widget races_form, races_toggles_form, races_label;
Widget races_ok_command;
Widget races_toggles[14], races_name;

/******************************************************************/
Widget about_dialog_shell;
Widget about_form, about_label, about_command;

/******************************************************************/
Widget help_copying_dialog_shell, help_keys_dialog_shell;


/******************************************************************/
Widget unit_select_dialog_shell;
Widget unit_select_form;
Widget unit_select_commands[100];
Widget unit_select_labels[100];
Widget unit_select_close_command;
Pixmap unit_select_pixmaps[100];
int unit_select_ids[100];
int unit_select_no;

void about_button_callback(Widget w, XtPointer client_data, 
			    XtPointer call_data);

void help_button_callback(Widget w, XtPointer client_data, 
			    XtPointer call_data);

void create_rates_dialog(void);
void create_races_dialog(void);
void create_about_dialog(void);
void create_help_dialog(Widget *shell);


int races_buttons_get_current(void);

void races_buttons_callback(Widget w, XtPointer client_data, 
			    XtPointer call_data);
void races_toggles_callback(Widget w, XtPointer client_data, 
			    XtPointer call_data);

void unit_select_callback(Widget w, XtPointer client_data, 
			    XtPointer call_data);



char *default_race_leader_names[] = {
  "Caesar",
  "Hammurabi",
  "Frederick",
  "Ramesses",
  "Lincoln",
  "Alexander",
  "Gandi",
  "Stalin",
  "Shaka",
  "Napoleon",
  "Montezuma",
  "Mao",
  "Elizabeth",
  "Genghis"
};

int is_showing_goverment_dialog;

int caravan_city_id;
int caravan_unit_id;

int diplomat_id;
int diplomat_target_id;

struct city *pcity_caravan_dest;
struct unit *punit_caravan;


/****************************************************************
...
*****************************************************************/
void notify_command_callback(Widget w, XtPointer client_data, 
			     XtPointer call_data)
{
  XtDestroyWidget(XtParent(XtParent(w)));
  XtSetSensitive(toplevel, TRUE);
}


/****************************************************************
...
*****************************************************************/
void popup_notify_dialog(char *headline, char *lines)
{
  Widget notify_dialog_shell, notify_form, notify_command;
  Widget notify_headline, notify_label;
  Dimension width, width2;
  
  notify_dialog_shell = XtCreatePopupShell(headline,
					   transientShellWidgetClass,
					   toplevel, NULL, 0);

  notify_form = XtVaCreateManagedWidget("notifyform", 
					 formWidgetClass, 
					 notify_dialog_shell, NULL);

  notify_headline=XtVaCreateManagedWidget("notifyheadline", 
			  labelWidgetClass, notify_form, 
			  XtNlabel, headline,
			  NULL);

  
  notify_label=XtVaCreateManagedWidget("notifylabel", 
			  labelWidgetClass, notify_form, 
			  XtNlabel, lines,
			  NULL);   

  notify_command = XtVaCreateManagedWidget("notifycommand", 
					   commandWidgetClass,
					   notify_form,
					   NULL);

  XtVaGetValues(notify_label, XtNwidth, &width, NULL);
  XtVaGetValues(notify_headline, XtNwidth, &width2, NULL);
  if(width>width2)
    XtVaSetValues(notify_headline, XtNwidth, width, NULL); 
  
  XtAddCallback(notify_command, XtNcallback, notify_command_callback, NULL);
  
  xaw_set_relative_position(toplevel, notify_dialog_shell, 25, 25);
  XtPopup(notify_dialog_shell, XtGrabNone);
  XtSetSensitive(toplevel, FALSE);
}



/****************************************************************
...
*****************************************************************/
void diplomat_bribe_yes_callback(Widget w, XtPointer client_data, 
				 XtPointer call_data)
{
  struct packet_diplomat_action req;

  req.action_type=DIPLOMAT_BRIBE;
  req.diplomat_id=diplomat_id;
  req.target_id=diplomat_target_id;

  send_packet_diplomat_action(&aconnection, &req);
  
  destroy_message_dialog(w);
}

/****************************************************************
...
*****************************************************************/
void diplomat_bribe_no_callback(Widget w, XtPointer client_data, 
				XtPointer call_data)
{
  destroy_message_dialog(w);
}



/****************************************************************
...
*****************************************************************/
void diplomat_bribe_callback(Widget w, XtPointer client_data, 
			     XtPointer call_data)
{
  char buf[512];
  struct unit *pdiplomat, *ptarget;
  
  destroy_message_dialog(w);
  
  if((pdiplomat=find_unit_by_id(diplomat_id)) && 
     (ptarget=find_unit_by_id(diplomat_target_id))) { 
    
    if(game.player_ptr->economic.gold>=ptarget->bribe_cost) {
      sprintf(buf, "Bribe unit for %d gold?\nTreasure %d gold.", 
	      ptarget->bribe_cost,
	      game.player_ptr->economic.gold);
      popup_message_dialog(toplevel, "diplomatbribedialog", buf,
			   diplomat_bribe_yes_callback, 0,
			   diplomat_bribe_no_callback, 0, 0);
    }
    else {
      sprintf(buf, "Bribing the unit costs %d gold.\nTreasure %d gold.", 
	      ptarget->bribe_cost,
	      game.player_ptr->economic.gold);
      popup_message_dialog(toplevel, "diplomatnogolddialog", buf,
			   diplomat_bribe_no_callback, 0, 
			   0);
    }
  
  }

}

/****************************************************************
...
*****************************************************************/
void diplomat_sabotage_callback(Widget w, XtPointer client_data, 
				XtPointer call_data)
{
  struct unit *pdiplomat;
  struct city *pcity;
  
  destroy_message_dialog(w);
  
  if((pdiplomat=find_unit_by_id(diplomat_id)) && 
     (pcity=find_city_by_id(diplomat_target_id))) { 
    struct packet_diplomat_action req;
    
    req.action_type=DIPLOMAT_SABOTAGE;
    req.diplomat_id=diplomat_id;
    req.target_id=diplomat_target_id;
    
    send_packet_diplomat_action(&aconnection, &req);
  }

}

/****************************************************************
...
*****************************************************************/
void diplomat_embassy_callback(Widget w, XtPointer client_data, 
			       XtPointer call_data)
{
  struct unit *pdiplomat;
  struct city *pcity;
  
  destroy_message_dialog(w);
  
  if((pdiplomat=find_unit_by_id(diplomat_id)) && 
     (pcity=find_city_by_id(diplomat_target_id))) { 
    struct packet_diplomat_action req;
    
    req.action_type=DIPLOMAT_EMBASSY;
    req.diplomat_id=diplomat_id;
    req.target_id=diplomat_target_id;
    
    send_packet_diplomat_action(&aconnection, &req);
  }

}

/****************************************************************
...
*****************************************************************/
void diplomat_steal_callback(Widget w, XtPointer client_data, 
			     XtPointer call_data)
{
  struct unit *pdiplomat;
  struct city *pcity;
  
  destroy_message_dialog(w);
  
  if((pdiplomat=find_unit_by_id(diplomat_id)) && 
     (pcity=find_city_by_id(diplomat_target_id))) { 
    struct packet_diplomat_action req;
    
    req.action_type=DIPLOMAT_STEAL;
    req.diplomat_id=diplomat_id;
    req.target_id=diplomat_target_id;
    
    send_packet_diplomat_action(&aconnection, &req);
  }

}


/****************************************************************
...
*****************************************************************/
void diplomat_incite_yes_callback(Widget w, XtPointer client_data, 
				 XtPointer call_data)
{
  struct packet_diplomat_action req;

  req.action_type=DIPLOMAT_INCITE;
  req.diplomat_id=diplomat_id;
  req.target_id=diplomat_target_id;

  send_packet_diplomat_action(&aconnection, &req);
  
  destroy_message_dialog(w);
}

/****************************************************************
...
*****************************************************************/
void diplomat_incite_no_callback(Widget w, XtPointer client_data, 
				XtPointer call_data)
{
  destroy_message_dialog(w);
}



/****************************************************************
...
*****************************************************************/
void diplomat_incite_callback(Widget w, XtPointer client_data, 
			      XtPointer call_data)
{
  char buf[512];
  struct unit *pdiplomat;
  struct city *pcity;

  destroy_message_dialog(w);
  
   if((pdiplomat=find_unit_by_id(diplomat_id)) && 
     (pcity=find_city_by_id(diplomat_target_id))) { 
      
      if(game.player_ptr->economic.gold>=pcity->incite_revolt_cost) {
	 sprintf(buf, "Incite a revolt for %d gold?\nTreasure %d gold.", 
		 pcity->incite_revolt_cost,
		 game.player_ptr->economic.gold);
	 popup_message_dialog(toplevel, "diplomatbribedialog", buf,
			      diplomat_incite_yes_callback, 0,
			      diplomat_incite_no_callback, 0, 0);
      }
    else {
      sprintf(buf, "Inciting a revolt costs %d gold.\nTreasure %d gold.", 
	      pcity->incite_revolt_cost,
	      game.player_ptr->economic.gold);
      popup_message_dialog(toplevel, "diplomatnogolddialog", buf,
			   diplomat_incite_no_callback, 0, 
			   0);
    }
  
  }

}



/****************************************************************
...
*****************************************************************/
void diplomat_cancel_callback(Widget w, XtPointer a, XtPointer b)
{
  destroy_message_dialog(w);
}



/****************************************************************
...
*****************************************************************/
void popup_diplomat_dialog(struct unit *punit, int dest_x, int dest_y)
{
  struct city *pcity;
  struct unit *ptunit;
  Widget shl;

  diplomat_id=punit->id;
  
  if((pcity=map_get_city(dest_x, dest_y)))
    diplomat_target_id=pcity->id;
  else if((ptunit=unit_list_get(&map_get_tile(dest_x, dest_y)->units, 0)))
    diplomat_target_id=ptunit->id;
  
  shl=popup_message_dialog(toplevel, "diplomatdialog", 
			   "Sir, the diplomat is waiting for your command",
			   diplomat_bribe_callback, 0,
			   diplomat_embassy_callback, 0,
			   diplomat_sabotage_callback, 0,
			   diplomat_steal_callback, 0,
			   diplomat_incite_callback, 0,
                           diplomat_cancel_callback, 0,
			   0);
  
  if(!diplomat_can_do_action(punit, DIPLOMAT_BRIBE, dest_x, dest_y))
    XtSetSensitive(XtNameToWidget(shl, "*button0"), FALSE);
  if(!diplomat_can_do_action(punit, DIPLOMAT_EMBASSY, dest_x, dest_y))
    XtSetSensitive(XtNameToWidget(shl, "*button1"), FALSE);
  if(!diplomat_can_do_action(punit, DIPLOMAT_SABOTAGE, dest_x, dest_y))
    XtSetSensitive(XtNameToWidget(shl, "*button2"), FALSE);
  if(!diplomat_can_do_action(punit, DIPLOMAT_STEAL, dest_x, dest_y))
    XtSetSensitive(XtNameToWidget(shl, "*button3"), FALSE);
  if(!diplomat_can_do_action(punit, DIPLOMAT_INCITE, dest_x, dest_y))
    XtSetSensitive(XtNameToWidget(shl, "*button4"), FALSE);
}




/****************************************************************
...
*****************************************************************/
void caravan_establish_trade_callback(Widget w, XtPointer client_data, 
				      XtPointer call_data)
{
  struct packet_unit_request req;
  req.unit_id=caravan_unit_id;
  req.city_id=caravan_city_id;
  req.name[0]='\0';
  send_packet_unit_request(&aconnection, &req, PACKET_UNIT_ESTABLISH_TRADE);
    
  destroy_message_dialog(w);
}


/****************************************************************
...
*****************************************************************/
void caravan_help_build_wonder_callback(Widget w, XtPointer client_data, 
					XtPointer call_data)
{
  struct packet_unit_request req;
  req.unit_id=caravan_unit_id;
  req.city_id=caravan_city_id;
  req.name[0]='\0';
  send_packet_unit_request(&aconnection, &req, PACKET_UNIT_HELP_BUILD_WONDER);

  destroy_message_dialog(w);
}


/****************************************************************
...
*****************************************************************/
void caravan_keep_moving_callback(Widget w, XtPointer client_data, 
				  XtPointer call_data)
{
  struct unit *punit;
  struct city *pcity;
  
  if((punit=find_unit_by_id(caravan_unit_id)) && 
     (pcity=find_city_by_id(caravan_city_id))) {
    struct unit req_unit;

    req_unit=*punit;
    req_unit.x=pcity->x;
    req_unit.y=pcity->y;
    send_unit_info(&req_unit);
  }
  destroy_message_dialog(w);
}


/****************************************************************
...
*****************************************************************/
void popup_caravan_dialog(struct unit *punit,
			  struct city *phomecity, struct city *pdestcity)
{
  Widget shl;
  char buf[128];
  
  sprintf(buf, "Your caravan from %s reaches the city of %s.\nWhat now?",
	  phomecity->name, pdestcity->name);
  
  caravan_city_id=pdestcity->id; /* callbacks need these */
  caravan_unit_id=punit->id;
  
  shl=popup_message_dialog(toplevel, "caravandialog", 
			   buf,
			   caravan_establish_trade_callback, 0,
			   caravan_help_build_wonder_callback, 0,
			   caravan_keep_moving_callback, 0,
			   0);
  
  if(!can_establish_trade_route(phomecity, pdestcity))
    XtSetSensitive(XtNameToWidget(shl, "*button0"), FALSE);
  
  if(!unit_can_help_build_wonder(punit, pdestcity))
    XtSetSensitive(XtNameToWidget(shl, "*button1"), FALSE);
}


/****************************************************************
...
*****************************************************************/
void goverment_callback(Widget w, XtPointer client_data, 
			    XtPointer call_data)
{
  struct packet_player_request packet;

  packet.goverment=(int)client_data;
  send_packet_player_request(&aconnection, &packet, PACKET_PLAYER_GOVERMENT);

  destroy_message_dialog(w);
  is_showing_goverment_dialog=0;
}


/****************************************************************
...
*****************************************************************/
void popup_goverment_dialog(void)
{
  if(!is_showing_goverment_dialog) {
    Widget shl;

    is_showing_goverment_dialog=1;
  
    shl=popup_message_dialog(toplevel, "govermentdialog", 
			     "Select goverment type:",
			     goverment_callback, G_DESPOTISM,
			     goverment_callback, G_MONARCHY,
			     goverment_callback, G_COMMUNISM,
			     goverment_callback, G_REPUBLIC,
			     goverment_callback, G_DEMOCRACY, 0);
    
    if(!can_change_to_goverment(game.player_ptr, G_DESPOTISM))
      XtSetSensitive(XtNameToWidget(shl, "*button0"), FALSE);
    if(!can_change_to_goverment(game.player_ptr, G_MONARCHY))
      XtSetSensitive(XtNameToWidget(shl, "*button1"), FALSE);
    if(!can_change_to_goverment(game.player_ptr, G_COMMUNISM))
      XtSetSensitive(XtNameToWidget(shl, "*button2"), FALSE);
    if(!can_change_to_goverment(game.player_ptr, G_REPUBLIC))
      XtSetSensitive(XtNameToWidget(shl, "*button3"), FALSE);
    if(!can_change_to_goverment(game.player_ptr, G_DEMOCRACY))
      XtSetSensitive(XtNameToWidget(shl, "*button4"), FALSE);
  }
}



/****************************************************************
...
*****************************************************************/
void revolution_callback_yes(Widget w, XtPointer client_data, 
			     XtPointer call_data)
{
  struct packet_player_request packet;

  send_packet_player_request(&aconnection, &packet, PACKET_PLAYER_REVOLUTION);
  
  destroy_message_dialog(w);
}

/****************************************************************
...
*****************************************************************/
void revolution_callback_no(Widget w, XtPointer client_data, 
			    XtPointer call_data)
{
  destroy_message_dialog(w);
}



/****************************************************************
...
*****************************************************************/
void popup_revolution_dialog(void)
{
  popup_message_dialog(toplevel, "revolutiondialog", 
		       "So you wanna revolution?",
		       revolution_callback_yes, 0,
		       revolution_callback_no, 0, 
		       0);
}


/****************************************************************
...
*****************************************************************/
Widget popup_message_dialog(Widget parent, char *dialogname, char *text, ...)
{
  va_list args;
  Widget dshell, dform, dlabel, button;
  Position x, y;
  Dimension width, height;
  void (*fcb)(Widget, XtPointer, XtPointer);
  XtPointer client_data;
  char button_name[512];
  int i;

  XtSetSensitive(parent, FALSE);
  
  dshell=XtCreatePopupShell(dialogname, transientShellWidgetClass,
			    parent, NULL, 0);
  
  dform=XtVaCreateManagedWidget("dform", formWidgetClass, dshell, NULL);
  
  dlabel=XtVaCreateManagedWidget("dlabel", labelWidgetClass, dform, 
				 XtNlabel, (XtArgVal)text,
				 NULL);   

  i=0;
  va_start(args, text);
  
  while((fcb=va_arg(args, void *))) {
    client_data=va_arg(args, XtPointer);
    sprintf(button_name, "button%d", i++);
    
    button=XtVaCreateManagedWidget(button_name, commandWidgetClass, 
				   dform, NULL);
    XtAddCallback(button, XtNcallback, fcb, client_data);
  }
  
  va_end(args);

  XtVaGetValues(parent, XtNwidth, &width, XtNheight, &height, NULL);
  XtTranslateCoords(parent, (Position) width/10, (Position) height/10,
		    &x, &y);
  XtVaSetValues(dshell, XtNx, x, XtNy, y, NULL);
  
  XtPopup(dshell, XtGrabNone);

  return dshell;
}

/****************************************************************
...
*****************************************************************/
void destroy_message_dialog(Widget button)
{
  XtSetSensitive(XtParent(XtParent(XtParent(button))), TRUE);

  XtDestroyWidget(XtParent(XtParent(button)));
}


/****************************************************************
popup the dialog 10% inside the main-window 
*****************************************************************/
void popup_unit_select_dialog(struct tile *ptile)
{
  int i;
  char buffer[512];

  XtSetSensitive(main_form, FALSE);

  unit_select_dialog_shell = XtCreatePopupShell("unitselectdialogshell", 
						transientShellWidgetClass,
						toplevel, NULL, 0);

  unit_select_form = XtVaCreateManagedWidget("unitselectform", 
					     formWidgetClass, 
					     unit_select_dialog_shell, NULL);


  for(i=0; i<unit_list_size(&ptile->units); i++) {
    struct unit *punit=unit_list_get(&ptile->units, i);
    struct unit_type *punittemp=get_unit_type(punit->type);
    struct city *pcity;
    
    unit_select_ids[i]=punit->id;

    pcity=city_list_find_id(&game.player_ptr->cities, punit->homecity);
    
    sprintf(buffer, "%s(%s)\n%s", 
	    punittemp->name, 
	    pcity ? pcity->name : "",
	    unit_activity_text(punit));

    unit_select_pixmaps[i]=XCreatePixmap(display, XtWindow(map_canvas), 
					 30, 30, display_depth);
    
    put_unit_pixmap(punit, unit_select_pixmaps[i], 0, 0);

    if(i==0) {
      unit_select_commands[0]=XtVaCreateManagedWidget("unitselectcommands0", 
						      commandWidgetClass,
						      unit_select_form,
						      XtNbitmap,
						      (XtArgVal)unit_select_pixmaps[i],
						      XtNsensitive,
						      punit->moves_left>0 &&
						      can_unit_do_activity(punit, ACTIVITY_IDLE),
						      NULL);

      unit_select_labels[0]=XtVaCreateManagedWidget("unitselectlabels0", 
						    labelWidgetClass, 
						    unit_select_form, 
						    XtNfromHoriz, unit_select_commands[0],
						    XtNlabel, (XtArgVal)buffer,
						    XtNborderWidth, 0,
						    NULL);   
    }
    else {
      unit_select_commands[i]=XtVaCreateManagedWidget("unitselectcommands1", 
						      commandWidgetClass,
						      unit_select_form,
						      XtNbitmap,
						      (XtArgVal)unit_select_pixmaps[i],
						      XtNfromVert, unit_select_commands[i-1],
						      XtNsensitive,
						      can_unit_do_activity(punit, ACTIVITY_IDLE),
						      NULL);

      unit_select_labels[i]=XtVaCreateManagedWidget("unitselectlabels1", 
						    labelWidgetClass, 
						    unit_select_form, 
						    XtNfromVert, unit_select_commands[i-1],
						    XtNfromHoriz, unit_select_commands[i],
						    XtNlabel, (XtArgVal)buffer,
						    XtNborderWidth, 0,
						    NULL);   
    }

    XtAddCallback(unit_select_commands[i], XtNcallback, unit_select_callback, NULL);
  }

  unit_select_no=i;


  unit_select_close_command=XtVaCreateManagedWidget("unitselectclosecommand", 
						    commandWidgetClass,
						    unit_select_form,
						    XtNfromVert, unit_select_commands[i-1],
						    XtNvertDistance, 30,
						    XtNlabel, "Close",
						    NULL);

  XtAddCallback(unit_select_close_command, XtNcallback, unit_select_callback, NULL);

  xaw_set_relative_position(toplevel, unit_select_dialog_shell, 25, 25);
  XtPopup(unit_select_dialog_shell, XtGrabNone);
}



/**************************************************************************
...
**************************************************************************/
void unit_select_callback(Widget w, XtPointer client_data, 
			    XtPointer call_data)
{
  int i;

  XtSetSensitive(main_form, TRUE);
  XtPopdown(unit_select_dialog_shell);

  for(i=0; i<unit_select_no; i++) {

    if(unit_select_commands[i]==w) {
      struct unit *punit=unit_list_find(&game.player_ptr->units, 
					unit_select_ids[i]);
      if(punit) {
	request_new_unit_activity(punit, ACTIVITY_IDLE);
	set_unit_focus(punit);
      }
      return;
    }
  }
}





/****************************************************************
popup the dialog 10% inside the main-window 
*****************************************************************/
void popup_races_dialog(void)
{
  Position x, y;
  Dimension width, height;

  XtSetSensitive(main_form, FALSE);

  if(!races_dialog_shell)
    create_races_dialog();

  XtVaGetValues(toplevel, XtNwidth, &width, XtNheight, &height, NULL);

  XtTranslateCoords(toplevel, (Position) width/10, (Position) height/10,
		    &x, &y);
  XtVaSetValues(races_dialog_shell, XtNx, x, XtNy, y, NULL);

  XtPopup(races_dialog_shell, XtGrabNone);
}

/****************************************************************
...
*****************************************************************/
void popdown_races_dialog(void)
{
  XtSetSensitive(main_form, TRUE);
  XtPopdown(races_dialog_shell);
}


/****************************************************************
...
*****************************************************************/
void create_races_dialog(void)
{
  int i;
  XtTranslations textfieldtranslations;

  races_dialog_shell = XtCreatePopupShell("racespopup", 
					  transientShellWidgetClass,
					  toplevel, NULL, 0);


  races_form = XtVaCreateManagedWidget("racesform", 
				       formWidgetClass, 
				       races_dialog_shell, NULL);   

  races_label = XtVaCreateManagedWidget("raceslabel", 
				       labelWidgetClass, 
				       races_form, NULL);  
  
  races_toggles_form = XtVaCreateManagedWidget("racestogglesform", 
					       formWidgetClass, 
					       races_form, 
					       XtNfromVert, races_label, 
					       NULL);   

  races_toggles[0]=XtVaCreateManagedWidget("racestoggle0", 
					   toggleWidgetClass, 
					   races_toggles_form,
					   NULL);
  for(i=1; i<14; i++) {
    char buf[64];
    sprintf(buf, "racestoggle%d", i);
    races_toggles[i]=XtVaCreateManagedWidget(buf, 
					     toggleWidgetClass, 
					     races_toggles_form,
					     XtNradioGroup, 
					     races_toggles[i-1], 
					     NULL);
  }

  races_name = XtVaCreateManagedWidget("racesname", 
				       asciiTextWidgetClass, 
				       races_form,
				       XtNfromVert, 
				       (XtArgVal)races_toggles_form,
				       XtNeditType, XawtextEdit,
				       XtNstring, "",
				       NULL);


  races_ok_command = XtVaCreateManagedWidget("racesokcommand", 
					     commandWidgetClass,
					     races_form,
					     XtNfromVert, 
					     (XtArgVal)races_name,
					     NULL);

  for(i=0; i<14; i++)
    XtAddCallback(races_toggles[i], XtNcallback, 
		  races_toggles_callback, (XtPointer) 0);


  XtAddCallback(races_ok_command, XtNcallback, races_buttons_callback, NULL);

  textfieldtranslations = 
    XtParseTranslationTable("<Key>Return: races-dialog-returnkey()");
  XtOverrideTranslations(races_name, textfieldtranslations);


  XtSetKeyboardFocus(races_form, races_name);
  
  XtRealizeWidget(races_dialog_shell);
}

/****************************************************************
...
*****************************************************************/
void races_dialog_returnkey(Widget w, XEvent *event, String *params,
			    Cardinal *num_params)
{
  x_simulate_button_click(XtNameToWidget(XtParent(w), "racesokcommand"));
}


/**************************************************************************
...
**************************************************************************/
void races_toggles_set_sensitive(int bits)
{
  int i, selected, mybits;

  mybits=bits;

  for(i=0; i<R_LAST; i++) {
    if(mybits&1)
      XtSetSensitive(races_toggles[i], FALSE);
    else
      XtSetSensitive(races_toggles[i], TRUE);
    mybits>>=1;
  }

  if((selected=races_buttons_get_current())==-1)
     return;

  if(bits&(1<<selected))
     XawToggleUnsetCurrent(races_toggles[0]);
}



/**************************************************************************
...
**************************************************************************/
void races_toggles_callback(Widget w, XtPointer client_data, 
			    XtPointer call_data)
{
  int i;

  for(i=0; i<14; i++)
    if(w==races_toggles[i]) {
      XtVaSetValues(races_name, XtNstring, 
		    default_race_leader_names[i], NULL);
      return;
    }
}

/**************************************************************************
...
**************************************************************************/
int races_buttons_get_current(void)
{
  int i;
  XtPointer dp, yadp;

  if(!(dp=XawToggleGetCurrent(races_toggles[0])))
    return -1;

  for(i=0; i<14; i++) {
    XtVaGetValues(races_toggles[i], XtNradioData, &yadp, NULL);
    if(dp==yadp)
      return i;
  }

  return -1;
}


/**************************************************************************
...
**************************************************************************/
void races_buttons_callback(Widget w, XtPointer client_data, 
			    XtPointer call_data)
{
  int selected;
  XtPointer dp;

  struct packet_alloc_race packet;

  if((selected=races_buttons_get_current())==-1) {
    append_output_window("You must select a race.");
    return;
  }

  XtVaGetValues(races_name, XtNstring, &dp, NULL);

  /* perform a minimum of sanity test on the name */
  packet.race_no=selected;
  strncpy(packet.name, (char*)dp, MAX_LENGTH_NAME);
  packet.name[MAX_LENGTH_NAME-1]='\0';
  
  if(!get_sane_name(packet.name)) {
    append_output_window("You must type a legal name.");
    return;
  }

  packet.name[0]=toupper(packet.name[0]);

  send_packet_alloc_race(&aconnection, &packet);
}




