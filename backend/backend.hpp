#pragma once
#include "../common.hpp"
#include "../renderer/renderer.hpp"
#include "../fiber/fiber.hpp"
#include "../services/injection/injection.hpp"

namespace VX {
	class backend
	{
	public:
		static void api();
		static void renderer();
		static void injection_find();
		static void discord();
	};
}