#pragma once

#include "sprite.h"
#include <halley/time/halleytime.h>

namespace Halley {
	class Particles {
	public:
		void update(Time t);
		void paint(Painter& painter);
	};
}