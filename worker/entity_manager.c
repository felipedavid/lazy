#include "mem.h"
#include "utils.h"
#include "offsets.h"
#include "entity_manager.h"
#include "logger.h"

Entity active_player;
Entity *entity_list;

const char *etype_str[] = {
	[ET_NONE]      = "None",
	[ET_ITEM]      = "Item",
	[ET_CONTAINER] = "Container",
	[ET_UNIT] 	   = "Unit",
	[ET_PLAYER]    = "Player",
	[ET_GAMEOBJ]   = "GameObject",
	[ET_DYNOBJ]    = "DynamicObject",
	[ET_CORPSE]    = "Corpse",
};

void log_entity(Entity ent) {
	log_info("Addr: 0x%x, Guid: %llu, Type: %s", ent, get_guid(ent), etype_str[get_type(ent)]);
}

void log_game_object(Entity ent) {
	log_info("Addr: 0x%x, Guid: %llu, Type: %s, Creator: %llu, Display: %d", 
		ent, get_guid(ent), etype_str[get_type(ent)], get_creator_guid(ent), get_display_id(ent));
}

void log_entity_list() {
	for (int i = 0; i < buf_len(entity_list); i++) {
		log_entity(entity_list[i]);
	}
}

void populate_entity_list() {
	buf_clear(entity_list);

	u8 *ent_mgr = read_pointer(ENTITY_MANAGER_PTR);
	Entity ent = read_pointer(ent_mgr + FIRST_ENTITY);

	while (ent && (((u32)ent & 1) == 0)) {
		if (is_active_player(ent)) {
			active_player = ent;
		} else {
			buf_push(entity_list, ent);
		}
		ent = read_pointer(ent + NEXT_ENTITY);
	}
}

u64 get_guid(Entity ent) {
	return read_u64(ent + ENTITY_GUID);
}

Entity_Type get_type(Entity ent) {
	return (Entity_Type) read_u32(ent + ENTITY_TYPE);
}

u32 get_casting_spell(Entity ent) {
	return read_u32(ent + CASTINGSPELL_ID);
}

u8 *get_descriptor(Entity ent) {
	return read_pointer(ent + ENTITY_DESCRIPTOR);
}

u64 get_creator_guid(Entity ent) {
	u8 *descriptor = get_descriptor(ent);
	return read_u64(descriptor + ENTITY_CREATOR_GUID);
}

u32 get_display_id(Entity ent) {
	u8 *descriptor = get_descriptor(ent);
	return read_u32(descriptor + ENTITY_DISPLAY_ID);
}

const char *get_name(Entity ent) {
	auto type = get_type(ent);
	switch (type) {
	case ET_UNIT:
		return (const char *) read_pointer(read_pointer(ent + UNIT_NAME));
	default:
		log_error("Not able to get name of objects of type %s\n", etype_str[type]);
	}
	return "<nil>";
}

b32 is_active_player(Entity ent) {
	u8 *entity_manager = read_pointer(ENTITY_MANAGER_PTR);
	u64 active_player_guid = read_u64(entity_manager + LOCAL_PLAYER_GUID);
	return get_guid(ent) == active_player_guid;
}
