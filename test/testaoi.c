#include <stdio.h>
#include "game/aoi.h"
#include "core/kn_string.h"
#include "core/systime.h"

struct player
{
	struct aoi_object aoi;
	string_t name; 
};


void enter_me(struct aoi_object *me,struct aoi_object *who)
{
	struct player* l_me = (struct player*)me;
	struct player* l_who = (struct player*)who;

	printf("%s enter %s\n",to_cstr(l_me->name),to_cstr(l_who->name));

}

void leave_me(struct aoi_object *me,struct aoi_object *who)
{
	struct player* l_me = (struct player*)me;
	struct player* l_who = (struct player*)who;

	printf("%s leave %s\n",to_cstr(l_me->name),to_cstr(l_who->name));	
}

uint8_t in_myscope(struct aoi_object *me,struct aoi_object *who){
	uint64_t distance = cal_distance_2D(&me->pos,&who->pos);
	if(distance <= 700)
		return 1;
	return 0;
}


static int map_size = 10000;
static int plycount = 100;

int main(){

	struct point2D t_left,b_right;
	t_left.x = 0;
	t_left.y = 0;
	b_right.x = map_size;
	b_right.y = map_size;

	struct aoi_map *l_map = create_map(plycount,500,700,&t_left,&b_right);

	
	struct player* players[plycount];
	int i = 0;
	for(; i <plycount ; ++i){
	
		struct player *ply = calloc(1,sizeof(*ply));
		char buf[32];
		snprintf(buf,32,"%d",i);	
		ply->aoi.cb_enter = enter_me;
		ply->aoi.cb_leave = leave_me;
		ply->aoi.in_myscope = in_myscope;
		ply->aoi.view_objs = new_bitset(plycount);
		ply->name = new_string(buf);
		players[i] = ply;
		enter_map(l_map,&ply->aoi,rand()%map_size,rand()%map_size);
	}

	printf("enter finish\n");	
	while(1){
		
		uint32_t idx = rand()%plycount;

		printf("-----------------%s begin move-----------\n",to_cstr(players[idx]->name));
		move_to(l_map,&players[idx]->aoi,rand()%map_size,rand()%map_size);
		sleepms(10);
		printf("-----------------%s end move-----------\n",to_cstr(players[idx]->name));
	}

/*
	struct player *ply1 = calloc(1,sizeof(*ply1));
	struct player *ply2 = calloc(1,sizeof(*ply2));
	struct player *ply3 = calloc(1,sizeof(*ply3));
	ply1->name = new_string("a");
	ply2->name = new_string("b");
	ply3->name = new_string("c");

	ply1->aoi.aoi_object_id = 0;
	ply2->aoi.aoi_object_id = 1;
	ply3->aoi.aoi_object_id = 2;

	struct point2D t_left,b_right;
	t_left.x = 0;
	t_left.y = 0;
	b_right.x = 150;
	b_right.y = 150;

	struct map *l_map = create_map(&t_left,&b_right,enter_me,leave_me);

	//a进入地图,坐标10,10
	enter_map(l_map,&ply1->aoi,10,10);

	//b进入地图,坐标60,10
	enter_map(l_map,&ply2->aoi,60,10);

	//C进入地图,坐标75,75
	enter_map(l_map,&ply3->aoi,75,75);

	printf("begin move\n");

	move_to(l_map,&ply3->aoi,60,10);

	move_to(l_map,&ply3->aoi,10,10);

	printf("c leave map\n");

	leave_map(l_map,&ply3->aoi);
	printf("a leave map\n");
	leave_map(l_map,&ply1->aoi);
	printf("b leave map\n");
	leave_map(l_map,&ply2->aoi);
*/
	return 0;
}
