#include "shader.hpp"
#include "../../STB/STBImage.hpp"
#include "../../STB/STBImageWrite.hpp"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

namespace VX {
	Shader g_Shader;

	void Shader::CreateImFont(ImFontAtlas* Atlas, ImFont*& Font, fs::path Path, float Size, ImFontConfig* Config) {
		Font = Atlas->AddFontFromFileTTF(Path.string().c_str(), Size, Config);
	}
	template <size_t DataSize>
	void Shader::CreateImFont(ImFontAtlas* Atlas, ImFont*& Font, uint8_t(&Data)[DataSize], float Size, ImFontConfig* Config) {
		Font = Atlas->AddFontFromMemoryTTF(Data, DataSize, Size, Config);
	}

	Shader* GetShader() { return &g_Shader; }
}