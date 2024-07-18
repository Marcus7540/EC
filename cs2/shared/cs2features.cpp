#include "cs2game.h"


//
// private data. only available for features.cpp
//

namespace cs2
{
//global variable for bhop
QWORD local_player_gb = 0;
BOOL bhop_enabled;
namespace features
{
	//
	// global shared variables
	// can be used at any features.cpp function
	//
	cs2::WEAPON_CLASS weapon_class;
	BOOL             b_aimbot_button;
	BOOL             b_triggerbot_button;
	BOOL			 b_incrosstrigger_button;


	//
	// triggerbot
	//
	static DWORD mouse_down_ms;
	static DWORD mouse_up_ms;

	//
	// rcs
	//
	static vec2  rcs_old_punch;

	//
	// aimbot
	//
	float target_distance;
	static DWORD lock_delay;
	static BOOL	 locked;
	static BOOL  locked_onshot;
	static BOOL  aimbot_active;
	static QWORD aimbot_target;
	static int   aimbot_bone;
	static DWORD aimbot_ms;
	static int local_player_index;
	//
	// infov event state
	//
	static BOOL event_state;

	DWORD defuse_time;
	BOOL oneshot_rising;
	QWORD diffusing_player;
	DWORD total_diffuse_time = 10000;

	BOOL bomb_planted;
	DWORD bomb_time;

	void reset(void)
	{
		lock_delay = 0;
		locked = 0;
		locked_onshot = 0;
		mouse_down_ms    = 0;
		mouse_up_ms      = 0;
		rcs_old_punch    = {};
		aimbot_active    = 0;
		aimbot_target    = 0;
		aimbot_bone      = 0;
		aimbot_ms        = 0;
	}

	inline void update_settings(void);

	
	static void has_target_event(QWORD local_player, QWORD target_player, float fov, vec2 aim_punch, vec3 aimbot_angle, vec2 view_angle, vec3 bone);


	static vec3 get_target_angle(QWORD local_player, vec3 position, DWORD num_shots, vec2 aim_punch);
	static void get_best_target(BOOL ffa, QWORD local_controller, QWORD local_player, DWORD num_shots, vec2 aim_punch, QWORD *target);
	static void standalone_rcs(DWORD shots_fired, vec2 vec_punch, float sensitivity);
	static void esp(QWORD local_player, QWORD target_player, vec3 head);
	static void render_normal_position(vec3 pos, int width, int height, float r, float g, float b, QWORD target_player);
	//static void render_normal_position(vec3 pos,int width, int height, float r, float g, float b); //can be used for things too
}
}

namespace config
{
	static BOOL  aimbot_visible_check;
	static BOOL  triggerbot_visible_check;
	static BOOL  visualize_hitbox;
	static BOOL  bhop;
	static BOOL  standalone_rcs;
	static BOOL  aimbot_enabled;
	static DWORD aimbot_button;
	static float aimbot_fov;
	static float aimbot_smooth;
	static BOOL  aimbot_multibone;
	static DWORD triggerbot_button;
	static DWORD incrosstriggerbot_button;
	static BOOL  trigger_aim;
	static float triggerbot_fov;
	static float triggerbot_smooth;
	static BOOL triggerbot_multibone;
	static BOOL  visuals_enabled;
	static BOOL  spotted_esp;
}

inline DWORD random_number(DWORD min, DWORD max)
{
	return min + cs2::engine::get_current_ms() % (max + 1 - min);
}

inline void cs2::features::update_settings(void)
{
	int crosshair_alpha = cs2::get_crosshairalpha();

	//
	// default global settings
	//
	//standlone rcs is still weird but better than default, improve it or don't use it
	//config::triggerbot_fov = 1; 
	//config::triggerbot_smooth = 1;
	config::triggerbot_multibone = 0;
	config::standalone_rcs = 0; //probably dont use
	config::aimbot_enabled = 1;
	config::aimbot_multibone = 1;
	config::spotted_esp = 0; //visible only esp
	config::aimbot_visible_check = 1; //aimbot visible check default on
	config::triggerbot_visible_check = 0;
	config::bhop = 0;
	config::trigger_aim = 1;
	config::aimbot_button = 317; //mouse1
	config::triggerbot_button = 320; //mouse5
	config::incrosstriggerbot_button = 82; //left alt
	config::aimbot_fov = 1.5f;
	config::aimbot_smooth = 3.0f;
	config::visuals_enabled = 1;
	config::visualize_hitbox = 0; //show aim location

#ifdef _KERNEL_MODE
	config::visuals_enabled = 1;
#else
	config::visuals_enabled = 1;
#endif

	switch (crosshair_alpha)
	{
	case 200:
		config::aimbot_multibone = 1;
		config::aimbot_fov = 3.5;
		config::aimbot_smooth = 3.5;
		config::triggerbot_button = 600;
		config::visualize_hitbox = 1;
		config::standalone_rcs = 0;
		break;
	case 201:
		config::aimbot_fov = 1.5;
		config::aimbot_smooth = 3;
		config::spotted_esp = 1;
		config::aimbot_enabled = 1;
		config::triggerbot_visible_check = 1;
		config::aimbot_visible_check = 1;
		break;
	case 202:
		config::aimbot_fov = 1.5;
		config::aimbot_smooth = 3;
		break;
	case 203:
		config::aimbot_fov = 2;
		config::aimbot_smooth = 2;
		break;
	case 204:
		config::aimbot_fov = 4;
		config::aimbot_smooth = 2;
		break;
	case 205:
		config::visuals_enabled = 1;
		config::aimbot_enabled = 0;
		break;
	case 206:
		config::spotted_esp = 1;
		config::triggerbot_visible_check = 1;
		config::aimbot_enabled = 0;
		break;
	case 207:
		config::aimbot_multibone = 0;
		config::aimbot_fov = 45;
		config::aimbot_smooth = 1.5;
		config::visualize_hitbox = 1;
		break;
	case 208:
		config::aimbot_fov = 2;
		config::aimbot_smooth = 4;
		config::triggerbot_fov = .75;
		break;
	case 209:
		config::aimbot_fov = 2;
		config::aimbot_smooth = 4;
		config::triggerbot_fov = .75;
		config::triggerbot_multibone = 1;
		break;
	case 210:
		config::aimbot_fov = 1;
		config::aimbot_smooth = 2;
		break;
	
	default:
	//	config::spotted_esp = 0;
	//	config::aimbot_visible_check = 1;
	//	config::triggerbot_visible_check = 0;
	//	config::bhop = 0;
	//	config::trigger_aim	  = 1;
	//	config::aimbot_button     = 317;
	//	config::triggerbot_button = 320;
	//	config::incrosstriggerbot_button = 82;
	//	config::aimbot_fov        = 1.5f;
	//	config::aimbot_smooth     = 3.0f;
	//	config::visuals_enabled   = 1;
	//	config::visualize_hitbox  = 0;
		break;
	}

	switch (weapon_class)
	{
	case cs2::WEAPON_CLASS::Zeus:
		config::trigger_aim = 1;
		config::aimbot_multibone = 1;
		break;
	case cs2::WEAPON_CLASS::Knife:
	case cs2::WEAPON_CLASS::Grenade:
		config::aimbot_enabled = 0;
		break;
	case cs2::WEAPON_CLASS::Pistol:
		config::aimbot_multibone = 0;
		config::trigger_aim = 1;
		break;
	case cs2::WEAPON_CLASS::C4:
		config::aimbot_enabled = 0;
		break;
	}

}

//
// this event is called from get best target/aimbot,
// when we have active target
//
static void cs2::features::has_target_event(QWORD local_player, QWORD target_player, float fov, vec2 aim_punch, vec3 aimbot_angle, vec2 view_angle, vec3 bone)
{
#ifndef __linux__
	UNREFERENCED_PARAMETER(local_player);
	UNREFERENCED_PARAMETER(target_player);
	UNREFERENCED_PARAMETER(aimbot_angle);
	UNREFERENCED_PARAMETER(bone);
#endif
	/*
	if (config::visuals_enabled)
	{

		if (fov < 5.0f)
		{
			//
			// net_graph( r, g , b ) 
			//
		}

		//
		// update ESP
		//
		
		if (event_state == 0)
		{
			esp(local_player, target_player, bone);
		}
		
	}*/

	if (b_triggerbot_button && mouse_down_ms == 0)
	{
		if (config::triggerbot_fov){
			config::aimbot_fov = config::triggerbot_fov;
		}
		if (config::triggerbot_smooth) {
			config::aimbot_smooth = config::triggerbot_smooth;
		}
		//for Zeusbot
		if (weapon_class == cs2::WEAPON_CLASS::Zeus)
		{
			config::aimbot_multibone = 1;
			vec3 target_origin = cs2::player::get_origin(target_player);
			vec3 local_origin = cs2::player::get_origin(local_player);
			target_distance = math::qsqrt(((target_origin.x - local_origin.x) * (target_origin.x - local_origin.x)) + ((target_origin.y - local_origin.y) * (target_origin.y - local_origin.y)));
		}
		else
		{
			target_distance = 0;
		}
		
		DWORD crosshair_id = cs2::player::get_crosshair_id(local_player);
		QWORD crosshair_target = cs2::entity::get_client_entity(crosshair_id);
		if (weapon_class == cs2::WEAPON_CLASS::Sniper or weapon_class == cs2::WEAPON_CLASS::Zeus)
		{
			if (crosshair_id == (DWORD)-1)
				return;

			
			if (crosshair_target == 0)
				return;

			if (cs2::player::get_health(crosshair_target) < 1)
				return;
		}
		else
		{
			cs2:config::aimbot_multibone = 0;
		}
		if (config::triggerbot_multibone)
		{
			config::aimbot_multibone = 1;
		}
		//team check
		if (cs2::gamemode::is_ffa() == 0 && cs2::player::get_team_num(crosshair_target) == cs2::player::get_team_num(local_player))
			return;

		//triggerbot crosshair check(accuracy boost)
		if (cs2::player::get_health(crosshair_target) < 1 && target_distance < 180)
			return;

		float accurate_shots_fl = -0.08f;
		if (weapon_class == cs2::WEAPON_CLASS::Pistol)
		{
			accurate_shots_fl = -0.04f;
		}

		//
		// accurate shots only
		//
		if (aim_punch.x > accurate_shots_fl)
		{
			typedef struct {
				float radius;
				vec3  min;
				vec3  max;
			} COLL;
			
			COLL coll = {
				2.800000f, {-0.200000f, 1.100000f,  0.000000f},  {3.600000f,  0.100000f, 0.000000f}
			};

			//these values could be optimized more i kinda guestimated them but i tested that they work
			switch (aimbot_bone)
			{
			case 1:
				coll = {
					6.800000f, {-0.300000f, 1.800000f,  0.000000f},  {10.600000f,  0.150000f, 0.000000f}
				};
				break;
				//body
			case 2:
				coll = {
					5.800000f, {-0.300000f, 1.800000f,  0.000000f},  {10.600000f,  0.150000f, 0.000000f}
				};
				break;
				//stomach
			case 3:
				coll = {
					6.800000f, {-0.300000f, 1.800000f,  0.000000f},  {10.600000f,  0.150000f, 0.000000f}
				};
				break;
			case 4:
				coll = {
					6.800000f, {-0.300000f, 1.800000f,  0.000000f},  {10.600000f,  0.150000f, 0.000000f}
				};
				break;
			case 5:
				coll = {
					3.600000f, {-0.300000f, 1.800000f,  0.000000f},  {3.000000f,  0.120000f, 0.000000f}
				};
				break;
			case 6: //head
				coll = {
					//	r						min											max							
					3.20000f, {-0.200000f, -1.100000f, 0.000000f},  {3.600000f,  1.100000f, 0.000000f}
				};
				bone.z += 2;
				break;
			}
			vec3 dir = math::vec_atd(vec3{view_angle.x, view_angle.y, 0});
			vec3 eye = cs2::player::get_eye_position(local_player);
			matrix3x4_t matrix{};
			matrix[0][3] = bone.x;
			matrix[1][3] = bone.y;
			matrix[2][3] = bone.z;


			
			if ( (math::vec_min_max(eye, dir,
				math::vec_transform(coll.min, matrix),
				math::vec_transform(coll.max, matrix),
				coll.radius)) && (target_distance < 180) or weapon_class == cs2::WEAPON_CLASS::Sniper or weapon_class == cs2::WEAPON_CLASS::Zeus or config::triggerbot_multibone or !(config::aimbot_enabled))
			{
				DWORD current_ms = cs2::engine::get_current_ms();
				if (current_ms > mouse_up_ms)
				{
					client::mouse1_down();
					mouse_up_ms   = 0;
					mouse_down_ms = random_number(30, 50) + current_ms;
				}
			}
		}
	}
	if (b_incrosstrigger_button && mouse_down_ms == 0)
	{
		DWORD crosshair_id = cs2::player::get_crosshair_id(local_player);
		//if (weapon_class == cs::WEAPON_CLASS::Sniper)
		//{
		if (crosshair_id == (DWORD)-1)
			return;

		QWORD crosshair_target = cs2::entity::get_client_entity(crosshair_id);
		if (crosshair_target == 0)
			return;

		if (cs2::gamemode::is_ffa() == 0 && cs2::player::get_team_num(crosshair_target) == cs2::player::get_team_num(local_player))
			return;
		if (weapon_class == cs2::WEAPON_CLASS::Zeus)
		{
			//return;//removeme
			vec3 target_origin = cs2::player::get_origin(target_player);
			vec3 local_origin = cs2::player::get_origin(local_player);
			target_distance = math::qsqrt(((target_origin.x - local_origin.x) * (target_origin.x - local_origin.x)) + ((target_origin.y - local_origin.y) * (target_origin.y - local_origin.y)));
		}
		else
		{
			target_distance = 0;
		}
		if (cs2::player::get_health(crosshair_target) < 1 && target_distance < 180)
			return;
		//}
		DWORD current_ms = cs2::engine::get_current_ms();
		if (current_ms > mouse_up_ms)
		{
			client::mouse1_down();
			mouse_up_ms = 0;
			mouse_down_ms = random_number(25, 35) + current_ms;
		}
	}

}

void cs2::features::run(void)
{

	//bomb timer esp
	if (cs2::offsets::get_BombPlanted() & 1)
	{
		DWORD current_ms = cs2::engine::get_current_ms();
		if (!bomb_planted)
		{
			bomb_time = current_ms + 41100;
			bomb_planted = 1;
		}
		QWORD sdl_window = cs2::sdl::get_window();
		if (sdl_window == 0)
			return;

		cs2::WINDOW_INFO window{};
		if (!cs2::sdl::get_window_info(sdl_window, &window))
			return;

		vec2 screen_size{};
		screen_size.x = (float)window.w;
		screen_size.y = (float)window.h;
		float timeleft = (float)(bomb_time - current_ms);

		int r = 0;
		int g = 100;
		int b = 255;

		if (timeleft < 11100)
		{
			r = 210;
			b = 120;
			g = 0;
		}
		if (timeleft < 6100)
		{
			r = 255;
			b = 0;
			g = 0;
		}

		int box_width = (int)((float)screen_size.x * (float)((float)(timeleft) / (float)41200));
		int y = (int)screen_size.y - 8;

#ifdef __linux__
		client::DrawfillRect((void*)0, 0, y, box_width, 8, (unsigned char)r, (unsigned char)g, (unsigned char)b);
#else
		QWORD sdl_window_data = cs2::sdl::get_window_data(sdl_window);
		if (sdl_window_data == 0)
			return;
		QWORD hwnd = cs2::sdl::get_hwnd(sdl_window_data);
		client::DrawFillRect((void*)hwnd, 0, y, box_width, 8, (unsigned char)r, (unsigned char)g, (unsigned char)b);
#endif
	}
	
	if (!cs2::offsets::get_BombPlanted() && (bomb_planted == 1))
	{
		bomb_planted = 0;
	}


	//reset diffuse esp
	if (!cs2::player::is_defusing(diffusing_player) && !oneshot_rising)
	{
		oneshot_rising = 1;
		diffusing_player = 0;
	}
	//bhop on or off
	if (config::bhop)
	{
		bhop_enabled = 1;
	}	
	else if (!config::bhop)
	{
		bhop_enabled = 0;
	}
	//
	// reset triggerbot input
	//
	if (mouse_down_ms)
	{
		DWORD current_ms = cs2::engine::get_current_ms();
		if (current_ms > mouse_down_ms)
		{
			client::mouse1_up();
			mouse_down_ms   = 0;
			mouse_up_ms     = random_number(30, 50) + current_ms;
		}
	}

	QWORD local_player_controller = cs2::entity::get_local_player_controller();
	if (local_player_controller == 0)
	{
	NOT_INGAME:
		if (mouse_down_ms)
		{
			client::mouse1_up();
			mouse_down_ms = 0;
		}
		reset();
		return;
	}

	QWORD local_player = cs2::entity::get_player(local_player_controller);

	if (local_player == 0)
	{
		goto NOT_INGAME;
	}
	
	local_player_gb = local_player;
	
	weapon_class = cs2::player::get_weapon_class(local_player);
	//only return for this if the player is alive
	if ((weapon_class == cs2::WEAPON_CLASS::Invalid) && (cs2::player::get_life_state(local_player) == 256))
	{
		return;
	}


	//
	// update cheat settings
	//
	update_settings();



	//
	// update buttons
	//
	b_triggerbot_button = cs2::input::is_button_down(config::triggerbot_button);
	b_incrosstrigger_button = cs2::input::is_button_down(config::incrosstriggerbot_button);
	b_aimbot_button = (cs2::input::is_button_down(config::aimbot_button) | (b_triggerbot_button & config::trigger_aim));
	

	/*
	// if we are holding triggerbot key, force head only
	//
	if (b_triggerbot_button)
	{;
		//LOG("MAP: %o \n", cs2::offsets::get_map());
		//config::aimbot_multibone = 0;
	}
	*/

	BOOL  ffa         = cs2::gamemode::is_ffa();
	DWORD num_shots   = cs2::player::get_shots_fired(local_player);
	vec2  aim_punch   = cs2::player::get_vec_punch(local_player);
	float sensitivity = cs2::mouse::get_sensitivity() * cs2::player::get_fov_multipler(local_player);

	if (config::standalone_rcs && !(aimbot_active))
	{
		standalone_rcs(num_shots, aim_punch, sensitivity);
	}
	
	if (!b_aimbot_button)
	{
		//
		// reset target  
		//
		locked = 0;
		locked_onshot = 0;
		aimbot_target = 0;
		aimbot_bone   = 0;
	}

	event_state = 0;

	QWORD best_target = 0;
	
	get_best_target(ffa, local_player_controller, local_player, num_shots, aim_punch, &best_target);	//its just better to always call this i always want wallhacks

	//
	// no previous target found, lets update target
	//

	if (aimbot_target == 0)
	{
		locked = 0;
		locked_onshot = 0;
		aimbot_bone = 0;
		aimbot_target = best_target;
	}
	else 
	{	if (!cs2::player::is_valid(aimbot_target, cs2::player::get_node(aimbot_target)))
		{
			aimbot_target = best_target;
			if (aimbot_target == 0)
			{
				locked = 0;
				locked_onshot = 0;
				aimbot_target = 0;
				aimbot_bone = 0;
				get_best_target(ffa, local_player_controller, local_player, num_shots, aim_punch, &aimbot_target);
			}
		}
	}

	aimbot_active = 0;

	//if (!config::aimbot_enabled || ((cs2::player::get_buy_menu(local_player)) && (cs2::player::get_buy_zone(local_player) == 257)))
	//{
	//	return;
	//}

	//
	// no valid target found
	//
	if (aimbot_target == 0)
	{
		return;
	}

	QWORD node = cs2::player::get_node(aimbot_target);
	if (node == 0)
	{
		return;
	}

	vec2 view_angle = cs2::player::get_viewangle(local_player);
	
	vec3  aimbot_angle{};
	vec3  aimbot_pos{};
	float aimbot_fov = 360.0f;

	/*
	if ((!b_aimbot_button) || ((config::aimbot_visible_check) && (~(cs2::player::get_spottedByMask(best_target) & (1 << (local_player_index - 1))))))
	{
		return;
	}
	*/

	if (config::triggerbot_visible_check && !cs2::player::is_visible(aimbot_target))
	{
		return;
	}
	if (!b_aimbot_button or config::aimbot_enabled == 0)
	{
		return;
	}

	if (config::aimbot_multibone)
	{
		//
		// use cached information for saving resources
		//
		if (aimbot_bone != 0)
		{
			vec3 pos{};
			if (!cs2::node::get_bone_position(node, aimbot_bone, &pos))
			{
				return;
			}
			aimbot_pos   = pos;
			aimbot_angle = get_target_angle(local_player, pos, num_shots, aim_punch);
			aimbot_fov   = math::get_fov(view_angle, aimbot_angle);
		}
		else
		{
			for (DWORD i = 2; i < 7; i++)
			{
				vec3 pos{};
				if (!cs2::node::get_bone_position(node, i, &pos))
				{
					continue;
				}

				vec3  angle = get_target_angle(local_player, pos, num_shots, aim_punch);
				float fov   = math::get_fov(view_angle, angle);

				if (fov < aimbot_fov)
				{
					aimbot_fov   = fov;
					aimbot_pos   = pos;
					aimbot_angle = angle;
					aimbot_bone  = i;
				}
			}
		}
	}
	else
	{
		vec3 head{};
		if (!cs2::node::get_bone_position(node, 6, &head))
		{
			return;
		}
		aimbot_pos   = head;
		aimbot_angle = get_target_angle(local_player, head, num_shots, aim_punch);
		aimbot_fov   = math::get_fov(view_angle, aimbot_angle);
	}

	if (config::visualize_hitbox)
	{
		render_normal_position(aimbot_pos,6, 6, 255, 0, 0, NULL);
	}

	if (event_state == 0)
	{
		if (aimbot_fov != 360.0f)
		{
			features::has_target_event(local_player, aimbot_target, aimbot_fov, aim_punch, aimbot_angle, view_angle, aimbot_pos);
		}
	}

	if (aimbot_fov > config::aimbot_fov)
	{
		//reset target if out of FOV
		aimbot_target = 0;
		aimbot_bone = 0;
		locked_onshot = 0;
		locked = 0;
		return;
	}

	aimbot_active = 1;

	vec3 angles{};
	angles.x = view_angle.x - aimbot_angle.x;
	angles.y = view_angle.y - aimbot_angle.y;
	angles.z = 0;
	math::vec_clamp(&angles);

	float x = (angles.y / sensitivity) / 0.022f;
	float y = (angles.x / sensitivity) / -0.022f;
	
	float smooth_x = 0.00f;
	float smooth_y = 0.00f;

	DWORD ms = 0;

	if (config::aimbot_smooth >= 1.0f)
	{
		if (qabs(x) > 1.0f)
		{
			if (smooth_x < x)
				smooth_x = smooth_x + 1.0f + (x / config::aimbot_smooth);
			else if (smooth_x > x)
				smooth_x = smooth_x - 1.0f + (x / config::aimbot_smooth);
			else
				smooth_x = x;
		}
		else
		{
			smooth_x = x;
		}

		if (qabs(y) > 1.0f)
		{
			if (smooth_y < y)
				smooth_y = smooth_y + 1.0f + (y / config::aimbot_smooth);
			else if (smooth_y > y)
				smooth_y = smooth_y - 1.0f + (y / config::aimbot_smooth);
			else
				smooth_y = y;
		}
		else
		{
			smooth_y = y;
		}
		ms = (DWORD)(config::aimbot_smooth / 100.0f) + 1;
		ms = ms * 16;
	}
	else	//rage aiming
	{
		//aiming
		if (locked == 0)
		{
			smooth_x = x;
			smooth_y = y;
			ms = 16;
		}
		//locked
		else if (locked == 1)
		{
			ms = 2;
		}
	}

	DWORD current_ms = cs2::engine::get_current_ms();
	if (current_ms - aimbot_ms > ms)
	{
		aimbot_ms = current_ms;
		client::mouse_move((int)smooth_x, (int)smooth_y);
		if (locked_onshot == 0)
		{ 
			lock_delay = current_ms + 32; 
			locked_onshot = 1;
		}
	}
}

static vec3 cs2::features::get_target_angle(QWORD local_player, vec3 position, DWORD num_shots, vec2 aim_punch)
{
	vec3 eye_position = cs2::node::get_origin(cs2::player::get_node(local_player));
	eye_position.z    += cs2::player::get_vec_view(local_player);

	//
	// which one is better???
	//
	// vec3 eye_position = cs2::player::get_eye_position(local_player);

	vec3 angle{};
	angle.x = position.x - eye_position.x;
	angle.y = position.y - eye_position.y;
	angle.z = position.z - eye_position.z;

	math::vec_normalize(&angle);
	math::vec_angles(angle, &angle);

	if (num_shots > 0)
	{
		if (weapon_class == cs2::WEAPON_CLASS::Sniper)
			goto skip_recoil;
		if (weapon_class == cs2::WEAPON_CLASS::Shotgun)
			goto skip_recoil;
		if (weapon_class == cs2::WEAPON_CLASS::Pistol)
		{
			if (num_shots < 2)
			{
				goto skip_recoil;
			}
		}
		angle.x -= aim_punch.x * 2.0f;
		angle.y -= aim_punch.y * 2.0f;
	}
skip_recoil:

	math::vec_clamp(&angle);

	return angle;
}

static void cs2::features::get_best_target(BOOL ffa, QWORD local_controller, QWORD local_player, DWORD num_shots, vec2 aim_punch, QWORD *target)
{
	vec2 va = cs2::player::get_viewangle(local_player);
	float best_fov = 360.0f;
	vec3  angle{};
	vec3  aimpos{};

	QWORD visible_player = 0;
	float visible_fov = 0.0f;
	vec3  visible_aimpos{};
	vec3  visible_angle{};

	for (int i = 1; i < 32; i++)
	{
		QWORD ent = cs2::entity::get_client_entity(i);
		if (ent == 0 || (ent == local_controller))
		{
			continue;
		}

		//
		// is controller
		//
		if (!cs2::entity::is_player(ent))
		{
			continue;
		}

		QWORD player = cs2::entity::get_player(ent);
		if (player == 0)
		{
			continue;
		}

		//if (cs2::player::get_health(player) < 1)
		//	return;

		if (ffa == 0)
		{
			if (cs2::player::get_team_num(player) == cs2::player::get_team_num(local_player))
			{
				continue;
			}
		}

		QWORD node = cs2::player::get_node(player);
		if (node == 0)
		{
			continue;
		}

		if (!cs2::player::is_valid(player, node))
		{
			continue;
		}

		vec3 head{};
		if (!cs2::node::get_bone_position(node, 6, &head))
		{
			continue;
		}
		//		if (config::visuals_enabled && b_visuals_button)

		if (config::visuals_enabled)
		{
			esp(local_player, player, head);
		}

		//	if (config::glow_enabled)
		//	{
		//		cs::player::write_detected_by_enemy_sensor_time(player, 86400.f);
		//	}

		vec3 best_angle = get_target_angle(local_player, head, num_shots, aim_punch);

		float fov = math::get_fov(va, *(vec3*)&best_angle);

		if (fov < best_fov)
		{
			best_fov = fov;
			*target = player;
			aimpos = head;
			angle = best_angle;
			if (cs2::player::is_visible(player) or b_triggerbot_button)
			{
				visible_aimpos = head;
				visible_angle = best_angle;
				visible_player = player;
				visible_fov = fov;
			}
		}
	}
	//if (visible_player != 1)
	//{
	best_fov = visible_fov;
	*target = visible_player;
	aimpos = visible_aimpos;
	angle = visible_angle;
	//}
	if (best_fov != 360.0f)
	{
		event_state = 1;
		features::has_target_event(local_player, *target, best_fov, aim_punch, angle, va, aimpos);
	}
}

// Random number generator state
UINT32 rand_state = 1;
// Random number generator
void initialize_random(UINT32 seed) {
	rand_state = seed;
}

float random_float(float min, float max) {
	// Linear Congruential Generator (LCG) constants
	const UINT32 a = 1103515245;
	const UINT32 c = 12345;
	const UINT32 m = 0x7fffffff;  // 2^31-1

	rand_state = (a * rand_state + c) & m;
	float random = (float)rand_state / (float)m;
	float diff = max - min;
	float r = random * diff;
	return min + r;
}

static void cs2::features::standalone_rcs(DWORD num_shots, vec2 vec_punch, float sensitivity)
{
	// Global variables for smoothing
	vec2 total_recoil_correction = { 0, 0 };
	vec2 remaining_recoil_correction = { 0, 0 };
	int smoothing_frames = 5;  // Number of frames to apply smoothing

	if (num_shots > 1)
	{
		float x = (vec_punch.x - rcs_old_punch.x) * -1.0f;
		float y = (vec_punch.y - rcs_old_punch.y) * -1.0f;

		// Calculate total mouse movement needed
		int mouse_angle_x = (int)(((y * 2.0f) / sensitivity) / -0.022f);
		int mouse_angle_y = (int)(((x * 2.0f) / sensitivity) / 0.022f);

		// Update total and remaining recoil correction
		total_recoil_correction.x += mouse_angle_x;
		total_recoil_correction.y += mouse_angle_y;

		// Smoothing and humanization
		float smoothing_factor = 1.0f / smoothing_frames;
		float humanization_factor = 0.5f; // Adjust for more or less humanization

		for (int i = 0; i < smoothing_frames; ++i)
		{
			if (!aimbot_active)
			{
				int smooth_mouse_angle_x = (int)((total_recoil_correction.x * smoothing_factor) + random_float(-humanization_factor, humanization_factor));
				int smooth_mouse_angle_y = (int)((total_recoil_correction.y * smoothing_factor) + random_float(-humanization_factor, humanization_factor));

				client::mouse_move(smooth_mouse_angle_x, smooth_mouse_angle_y);

				remaining_recoil_correction.x -= smooth_mouse_angle_x;
				remaining_recoil_correction.y -= smooth_mouse_angle_y;
			}
		}
		// Update remaining recoil correction for the next frame
		total_recoil_correction.x = remaining_recoil_correction.x;
		total_recoil_correction.y = remaining_recoil_correction.y;

	}
	rcs_old_punch = vec_punch;
}

static void cs2::features::esp(QWORD local_player, QWORD target_player, vec3 head)
{
	QWORD sdl_window = cs2::sdl::get_window();
	if (sdl_window == 0)
		return;


	cs2::WINDOW_INFO window{};
	if (!cs2::sdl::get_window_info(sdl_window, &window))
		return;

	/*
	float view = cs2::player::get_vec_view(local_player) - 10.0f;

	
	vec3 bottom;
	bottom.x = head.x;
	bottom.y = head.y;
	bottom.z = head.z - view;

	vec3 top;
	top.x = bottom.x;
	top.y = bottom.y;
	top.z = bottom.z + view;
	

	view_matrix_t view_matrix = cs2::engine::get_viewmatrix();


	vec2 screen_size;
	screen_size.x = window.w;
	screen_size.y = window.h;


	vec3 screen_bottom, screen_top;
	if (!math::world_to_screen(screen_size, bottom, screen_bottom, view_matrix) || !math::world_to_screen(screen_size, top, screen_top, view_matrix))
	{
		return;
	}

	float height  = (screen_bottom.y - screen_top.y) / 8.f;
	float width   = (screen_bottom.y - screen_top.y) / 14.f;

	int x = (int)((screen_top.x - (width  / 2.0f)) + window.x);
	int y = (int)((screen_top.y - (height / 2.0f)) + window.y);
	int w = (int)width;
	int h = (int)height;
	
	if (x > (int)(window.x + screen_size.x - (w)))
	{
		return;
	}
	else if (x < window.x)
	{
		return;
	}

	if (y > (int)(screen_size.y + window.y - (h)))
	{
		return;
	}
	else if (y < window.y)
	{
		return;
	}

	float target_health = ((float)cs2::player::get_health(target_player) / 100.0f) * 255.0f;
	float r = 255.0f - target_health;
	float g = target_health;
	float b = 0.00f;
	
#ifdef __linux__
	client::DrawFillRect((void *)0, x, y, w, h, (unsigned char)r, (unsigned char)g, (unsigned char)b);
#else
	QWORD sdl_window_data = cs2::sdl::get_window_data(sdl_window);
	if (sdl_window_data == 0)
		return;
	QWORD hwnd = cs2::sdl::get_hwnd(sdl_window_data);
	client::DrawFillRect((void *)hwnd, x, y, w, h, (unsigned char)r, (unsigned char)g, (unsigned char)b);
#endif
	*/
		
#ifndef __linux__
	UNREFERENCED_PARAMETER(local_player);
	UNREFERENCED_PARAMETER(head);
#endif

	if (!(cs2::player::get_health(local_player)) < 1 )
	{
		if (config::spotted_esp && !(cs2::player::is_visible(target_player))) //fix if dead
		{
			return;
		}
	}
	vec3 origin = cs2::player::get_origin(target_player);
	vec3 top_origin = origin;
	
	//get crouched flag and set correct box height
	BOOL crouched;
	if (cs2::player::get_flags(target_player) & (1 << 1))
	{
		top_origin.z += 57;
		crouched = 1;
	}
	else
	{
		top_origin.z += 75;
		crouched = 0;
	}


	vec3 screen_bottom, screen_top;
	view_matrix_t view_matrix = cs2::engine::get_viewmatrix();

	vec2 screen_size{};
	screen_size.x = (float)window.w;
	screen_size.y = (float)window.h;


	if (!math::world_to_screen(screen_size, origin, screen_bottom, view_matrix) || !math::world_to_screen(screen_size, top_origin, screen_top, view_matrix))
	{
		return;
	}
	
	//defusal progress esp
	if (cs2::player::is_defusing(target_player))
	{
		diffusing_player = target_player;

		DWORD currentms = cs2::engine::get_current_ms();

		if (cs2::entity::has_defuser(target_player) & oneshot_rising)
		{
			total_diffuse_time = 4000;
			defuse_time = currentms + total_diffuse_time;
			oneshot_rising = 0;
		}
		else if (oneshot_rising)
		{
			total_diffuse_time = 10000;
			defuse_time = currentms + total_diffuse_time;
			oneshot_rising = 0;
		}
		if (!oneshot_rising)
		{
			float bar_length = ((float)(defuse_time - currentms) / (float)total_diffuse_time);
			int width = (int)((float)40 * (bar_length));
			head.z += 20;
			render_normal_position(head, width, 3, 50, 50, 255, NULL);
		}
	}

	float target_health = ((float)cs2::player::get_health(target_player) / 100.0f) * 255.0f;
	float r = 255.0f - target_health;
	float g = target_health;
	float b = 0.00f;


	if (config::visuals_enabled == 2)
	{
		if (aimbot_target == target_player)
		{
			g = 144.0f;
			b = 255.0f;
		}
	}



	int box_height = (int)(screen_bottom.y - screen_top.y);
	int box_width;



	//correct bounding width without calculating for it 57/75(ratio of box heights) * 2(box width) = 1.52
	if (crouched)
	{
		box_width = (int)((float)box_height / (float)1.52);
	}
	else
	{
		box_width = box_height / 2;
	}
	int x = (int)window.x + (int)(screen_top.x - box_width / 2);
	int y = (int)window.y + (int)(screen_top.y);

	if (x > (int)(window.x + screen_size.x - (box_width)))
	{
		return;
	}
	else if (x < window.x)
	{
		return;
	}

	if (y > (int)(screen_size.y + window.y - (box_height)))
	{
		return;
	}
	else if (y < window.y)
	{
		return;
	}
	int barWidth = 4.5;
	int barHeight = box_height;

	int greenBarHeight = static_cast<int>((float)cs2::player::get_health(target_player) / 100.0f * barHeight);
	int redOverlayHeight = barHeight - greenBarHeight;
#ifdef __linux__
	client::DrawRect((void *)0, x, y, box_width, box_height, (unsigned char)r, (unsigned char)g, (unsigned char)b);
#else


	QWORD sdl_window_data = cs2::sdl::get_window_data(sdl_window);
	if (sdl_window_data == 0)
		return;
	QWORD hwnd = cs2::sdl::get_hwnd(sdl_window_data);
	#ifdef _KERNEL_MODE
	wchar_t name[] = L"Testing"; // Array (modifiable)
	client::DrawText((void*)hwnd, x, y, name);
	#endif
	client::DrawRect((void*)hwnd, x - barWidth, y, barWidth, box_height, 0, 255, 0);
	client::DrawRect((void*)hwnd, x - barWidth, y, barWidth, redOverlayHeight, 255, 0, 0);

	client::DrawRect((void *)hwnd, x, y, box_width, box_height, (unsigned char)r, (unsigned char)g, (unsigned char)b);
#endif
}

static void cs2::features::render_normal_position(vec3 pos, int width, int height, float r, float g, float b, QWORD target_player)
{
	QWORD sdl_window = cs2::sdl::get_window();

	if (sdl_window == 0)
		return;

	cs2::WINDOW_INFO window{};

	if (!cs2::sdl::get_window_info(sdl_window, &window))
		return;

	vec3 top_origin = pos;
	vec3 origin = pos;
	top_origin.z += height;

	vec3 screen_bottom, screen_top;
	view_matrix_t view_matrix = cs2::engine::get_viewmatrix();

	vec2 screen_size{};
	screen_size.x = (float)window.w;
	screen_size.y = (float)window.h;

	if (!math::world_to_screen(screen_size, origin,/*out*/ screen_bottom, view_matrix) || !math::world_to_screen(screen_size, top_origin, /*out*/ screen_top, view_matrix))
	{
		return;
	}

	int box_height = (int)(screen_bottom.y - screen_top.y);
	int box_width = box_height * (width / height);

	int x = (int)window.x + (int)(screen_top.x - box_width / 2);
	int y = (int)window.y + (int)(screen_top.y);

	//if box off screen dont draw
	if (x > (int)(window.x + screen_size.x - (box_width)))
	{
		return;
	}
	else if (x < window.x)
	{
		return;
	}
	if (y > (int)(screen_size.y + window.y - (box_height)))
	{
		return;
	}
	else if (y < window.y)
	{
		return;
	}
	/*
	//int barWidth = 4.5;
	//int barHeight = box_height;

	//int greenBarHeight = static_cast<int>((float)cs2::player::get_health(target_player) / 100.0f * barHeight);
	//int redOverlayHeight = barHeight - greenBarHeight;
	*/
#ifdef __linux__
	client::DrawfillRect((void*)0, x, y, box_width, box_height, (unsigned char)r, (unsigned char)g, (unsigned char)b);
#else
	QWORD sdl_window_data = cs2::sdl::get_window_data(sdl_window);
	if (sdl_window_data == 0)
		return;
	QWORD hwnd = cs2::sdl::get_hwnd(sdl_window_data);
	/*
	//#ifdef _KERNEL_MODE
	//wchar_t name[] = L"Testing"; // Array (modifiable)
	//client::DrawText((void*)hwnd, x, y, name);
	//#endif

	//client::DrawRect((void*)hwnd, x - barWidth, y, barWidth, box_height, 0, 255, 0);
	//client::DrawRect((void*)hwnd, x - barWidth, y, barWidth, redOverlayHeight, 255, 0, 0);
	*/

	client::DrawFillRect((void*)hwnd, x, y, box_width, box_height, (unsigned char)r, (unsigned char)g, (unsigned char)b);


#endif
}
