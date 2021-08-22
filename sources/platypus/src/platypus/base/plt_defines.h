#pragma once

// Toggle debug timer console output
#define PLT_DEBUG_TIMERS 0

// Toggle rendering thread IDs by colour
#define PLT_DEBUG_RASTER_THREAD_ID 1

// Toggle debug rendering of collision shapes 
#define PLT_DEBUG_COLLIDERS 0

// Maximum number of collisions that an object can receive per-update.
// Any additional collisions are ignored.
#define PLT_MAXIMUM_COLLISIONS_PER_OBJECT 32