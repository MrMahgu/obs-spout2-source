#pragma once
#include <string>
#include <atomic>

#include <obs-module.h>
#include <graphics/graphics.h>

#include "inc/SpoutSenderNames.h"

#define OBS_PLUGIN "spout2-texture-source"
#define OBS_PLUGIN_ "spout2_texture_source"
#define OBS_PLUGIN_VERSION_MAJOR 0
#define OBS_PLUGIN_VERSION_MINOR 0
#define OBS_PLUGIN_VERSION_RELEASE 1
#define OBS_PLUGIN_VERSION_STRING "0.0.1"
#define OBS_PLUGIN_LANG "en-US"
#define OBS_PLUGIN_COLOR_SPACE GS_RGBA_UNORM

#define OBS_SETTING_UI_FILTER_NAME "mahgu.spout2texturesrc.ui.filter_title"
#define OBS_SETTING_UI_SENDER_NAME "mahgu.spout2texturesrc.ui.sender_name"
#define OBS_SETTINGS_UI_BUTTON_TITLE "mahgu.spout2texturesrc.ui.button_title"
#define OBS_SETTING_DEFAULT_SENDER_NAME \
	"mahgu.spout2texturesrc.default.sender_name"

#define obs_log(level, format, ...) \
	blog(level, "[spout2-texture-source] " format, ##__VA_ARGS__)

#define error(format, ...) obs_log(LOG_ERROR, format, ##__VA_ARGS__)
#define warn(format, ...) obs_log(LOG_WARNING, format, ##__VA_ARGS__)
#define info(format, ...) obs_log(LOG_INFO, format, ##__VA_ARGS__)
#define debug(format, ...) obs_log(LOG_DEBUG, format, ##__VA_ARGS__)

bool obs_module_load(void);
void obs_module_unload();

spoutSenderNames spout;

namespace Spout2SourceTexture {
// Texture stuff
namespace SharedTexture {
inline static bool open(void *data,
					      bool currentTextureIsValid,
					      uint32_t handle, uint32_t width,
					      uint32_t height);
inline static void destroy(void *data);
inline static void render(void *data, gs_effect_t *effect);

} // namespace Texture

// DEBUG stuff

void report_version();

// OBS plugin stuff

static const char *filter_get_name(void *unused);
static obs_properties_t *filter_get_properties(void *unused);
static void filter_get_defaults(obs_data_t *defaults);

static void *filter_create(obs_data_t *settings, obs_source_t *source);
static void filter_destroy(void *data);
static void filter_update(void *data, obs_data_t *settings);
static void filter_video_render(void *data, gs_effect_t *effect);

static uint32_t filter_get_width(void *data);
static uint32_t filter_get_height(void *data);

struct filter {
	obs_source_t *context;

	const char *setting_sender_name; // realtime setting

	std::string sender_name; // spout sendername

	gs_texture_t *texture;

	uint32_t width;
	uint32_t height;

	std::atomic<bool> can_render;
};

struct obs_source_info create_source_info()
{
	struct obs_source_info source_info = {};

	source_info.id = OBS_PLUGIN_;
	source_info.type = OBS_SOURCE_TYPE_INPUT;
	source_info.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_CUSTOM_DRAW;

	source_info.get_name = filter_get_name;
	source_info.get_defaults = filter_get_defaults;
	source_info.get_properties = filter_get_properties;

	source_info.create = filter_create;
	source_info.destroy = filter_destroy;
	source_info.update = filter_update;

	source_info.video_render = filter_video_render;

	source_info.get_width = filter_get_width;
	source_info.get_height = filter_get_height;

	return source_info;
}

} // namespace Spout2SourceTexture
