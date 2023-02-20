#include "spout2-texture-source.h"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(OBS_PLUGIN, OBS_PLUGIN_LANG)

namespace Spout2SourceTexture {

static const char *filter_get_name(void *unused)
{
	UNUSED_PARAMETER(unused);
	return obs_module_text(OBS_SETTING_UI_FILTER_NAME);
}

static obs_properties_t *filter_get_properties(void *unused)
{
	UNUSED_PARAMETER(unused);

	auto props = obs_properties_create();

	obs_properties_set_flags(props, OBS_PROPERTIES_DEFER_UPDATE);

	return props;
}

static void filter_get_defaults(obs_data_t *defaults)
{
	obs_data_set_default_string(
		defaults, OBS_SETTING_UI_SENDER_NAME,
		obs_module_text(OBS_SETTING_DEFAULT_SENDER_NAME));
}

static void filter_update(void *data, obs_data_t *settings)
{
	UNUSED_PARAMETER(settings);

	auto filter = (struct filter *)data;

	if (!filter->context)
		return;
}

static void filter_video_render(void *data, gs_effect_t *effect)
{
	UNUSED_PARAMETER(effect);

	auto filter = (struct filter *)data;

	if (!filter->context)
		return;

	// process here?
	HANDLE handle;
	DWORD format;
	uint32_t width, height;

	//blog(LOG_INFO, filter->sender_name.c_str());

	auto res = spout.CheckSender(filter->sender_name.c_str(), width, height,
				     handle, format);

	// returns true if everything is valid (the same as before)
	bool canRender = (res && filter->texture && filter->width == width &&
			  filter->height == height);

	// Check width and height and peace out if its useless
	if (width == 0 || height == 0) {
		if (filter->texture) {
			gs_texture_destroy(filter->texture);
			filter->texture = nullptr;
		}
		return;
	}

	// If we can't render, and our texture is still active, destroy it
	if (!canRender && filter->texture) {
		gs_texture_destroy(filter->texture);
		filter->texture = nullptr;
	}

	// If we can't render but we're here, we need to open the shared handle again
	// We also need to update the width & height
	if (!canRender && !filter->texture) {

		gs_texture_t *tex =
			gs_texture_open_shared((uint32_t)(uintptr_t)handle);

		if (tex) {

			filter->texture = tex;

			tex = nullptr;

			filter->width = width;
			filter->height = height;
		}
	}

	// Failed to open the shared texture
	if (!filter->texture) {
		info("could not open spout2 sender shared resource");
		return;
	}

	// NOTE :: canRender is meaningless now

	effect = obs_get_base_effect(OBS_EFFECT_DEFAULT);

	const bool previous = gs_framebuffer_srgb_enabled();

	gs_enable_framebuffer_srgb(true);

	gs_blend_state_push();
	gs_blend_function(GS_BLEND_ONE, GS_BLEND_INVSRCALPHA);

	gs_eparam_t *const image = gs_effect_get_param_by_name(effect, "image");

	gs_effect_set_texture(image, filter->texture);

	// GS_FLIP_V
	while (gs_effect_loop(effect, "DrawSrgbDecompress"))
		gs_draw_sprite(filter->texture, 0, 0, 0);

	gs_blend_state_pop();

	gs_enable_framebuffer_srgb(previous);
}

static uint32_t filter_get_width(void *data)
{
	auto filter = (struct filter *)data;

	if (!filter->width)
		return 0;

	return filter->width;
}

static uint32_t filter_get_height(void *data)
{
	auto filter = (struct filter *)data;

	if (!filter->height)
		return 0;

	return filter->height;
}

static void *filter_create(obs_data_t *settings, obs_source_t *source)
{
	auto filter =
		(struct filter *)bzalloc(sizeof(Spout2SourceTexture::filter));

	// Fake some shit
	filter->width = 0;
	filter->height = 0;
	filter->sender_name = std::string("test");

	// Setup the obs context
	filter->context = source;

	// force an update
	filter_update(filter, settings);

	return filter;
}

static void filter_destroy(void *data)
{
	auto filter = (struct filter *)data;

	if (filter) {

		obs_enter_graphics();

		// Destroy texture if still exits
		if (filter->texture) {
			gs_texture_destroy(filter->texture);
			filter->texture = nullptr;
		}

		// Release spout sender regardless
		// TODO Debug spoutReleaseSender always
		spout.ReleaseSenderName(filter->sender_name.c_str());

		obs_leave_graphics();

		bfree(filter);
	}
}

// Writes a simple log entry to OBS
static inline void report_version()
{

#ifdef DEBUG
	info("you can haz spout2-texture tooz (Version: %s)",
	     OBS_PLUGIN_VERSION_STRING);
#else
	info("obs-spout2-filter [mrmahgu] - version %s",
	     OBS_PLUGIN_VERSION_STRING);
#endif
}

} // namespace Spout2Texture

bool obs_module_load(void)
{
	auto filter_info = Spout2SourceTexture::create_source_info();

	obs_register_source(&filter_info);

	Spout2SourceTexture::report_version();

	return true;
}

void obs_module_unload() {}
