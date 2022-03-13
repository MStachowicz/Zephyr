#include "DrawCall.hpp"

std::pair<std::string, Material> Material::get(const Material::Preset &pPreset)
{
	return presets[util::toIndex(pPreset)];
}

const std::array<std::pair<std::string, Material>, util::toIndex(Material::Preset::Count)> Material::presets =
{{
    {"Emerald", 		{{0.0215f, 0.1745f, 0.0215f}, {0.07568f, 0.61424f, 0.07568f}, {0.633f, 0.727811f, 0.633f}, 76.8f}},
	{"Jade", 			{{0.135f, 0.2225f, 0.1575f}, {0.54f, 0.89f, 0.63f}, {0.316228f, 0.316228f, 0.316228f}, 12.8f}},
	{"Obsidian",		{{0.05375f, 0.05f, 0.06625f}, {0.18275f, 0.17f, 0.22525f}, {0.332741f, 0.328634f, 0.346435f}, 38.4f}},
	{"Pearl", 			{{0.25f, 0.20725f, 0.20725f}, {1.f, 0.829f, 0.829f}, {0.296648f, 0.296648f, 0.296648f}, 112.64f}},
	{"Ruby", 			{{0.1745f, 0.01175f, 0.01175f}, {0.61424f, 0.04136f, 0.04136f}, {0.727811f, 0.626959f, 0.626959f}, 76.8f}},
	{"Turquoise",		{{0.1f, 0.18725f, 0.1745f}, {0.396f, 0.74151f, 0.69102f}, {0.297254f, 0.30829f, 0.306678f}, 12.8f}},
	{"Brass",			{{0.329412f, 0.223529f, 0.027451f}, {0.780392f, 0.568627f, 0.113725f}, {0.992157f, 0.941176f, 0.807843f}, 27.89743616f}},
	{"Bronze",			{{0.2125f, 0.1275f, 0.054f}, {0.714f, 0.4284f, 0.18144f}, {0.393548f, 0.271906f, 0.166721f},25.6f}},
	{"Chrome",			{{0.25f, 0.25f, 0.25}, {0.4f, 0.4f, 0.4f}, {0.774597f, 0.774597f, 0.774597f}, 76.8f}},
	{"Copper",			{{0.19125f, 0.0735f, 0.0225f}, {0.7038f, 0.27048f, 0.0828f}, {0.256777f, 0.137622f, 0.086014f}, 12.8f}},
	{"Gold",			{{0.24725f, 0.1995f, 0.0745f}, {0.75164f, 0.60648f, 0.22648f}, {0.628281f, 0.555802f, 0.366065f}, 51.2f}},
	{"Silver",			{{0.19225f, 0.19225f, 0.19225f}, {0.50754f, 0.50754f, 0.50754f}, {0.508273f, 0.508273f, 0.508273f}, 51.2f}},
	{"Black Plastic",	{{0.0f, 0.0f, 0.0f}, {0.01f, 0.01f, 0.01f}, {0.50f, 0.50f, 0.50f}, 32.f}},
	{"Cyan Plastic",	{{0.0f, 0.1f, 0.06f}, {0.0f, 0.50980392f, 0.50980392f}, {0.50196078f, 0.50196078f, 0.50196078f},32.f}},
	{"Green Plastic",	{{0.0f, 0.0f, 0.0f}, {0.1f, 0.35f, 0.1f}, {0.45f, 0.55f, 0.45f}, 32.f}},
	{"Red Plastic",		{{0.0f, 0.0f, 0.0f}, {0.5f, 0.0f, 0.0f}, {0.7f, 0.6f, 0.6f}, 32.f}},
	{"White Plastic",	{{0.0f, 0.0f, 0.0f}, {0.55f, 0.55f, 0.55f}, {0.70f, 0.70f, 0.70f}, 32.f}},
	{"Yellow Plastic",	{{0.0f, 0.0f, 0.0f}, {0.5f, 0.5f, 0.0f}, {0.60f, 0.60f, 0.50f}, 32.f}},
	{"Black Rubber",	{{0.02f, 0.02f, 0.02f}, {0.01f, 0.01f, 0.01f}, {0.4f, 0.4f, 0.4f}, 10.f}},
	{"Cyan Rubber",		{{0.0f, 0.05f, 0.05f}, {0.4f, 0.5f, 0.5f}, {0.04f, 0.7f, 0.7f}, 10.f}},
	{"Green Rubber",	{{0.0f, 0.05f, 0.0f}, {0.4f, 0.5f, 0.4f}, {0.04f, 0.7f, 0.04f}, 10.f}},
	{"Red Rubber",		{{0.05f, 0.0f, 0.0f}, {0.5f, 0.4f, 0.4f}, {0.7f, 0.04f, 0.04f}, 10.f}},
	{"White Rubber",	{{0.05f, 0.05f, 0.05f}, {0.5f, 0.5f, 0.5f}, {0.7f, 0.7f, 0.7f},	10.f}},
	{"Yellow Rubber",	{{0.05f, 0.05f, 0.0f}, {0.5f, 0.5f, 0.4f}, {0.7f, 0.7f, 0.04f}, 10.f}}
}};