#pragma once


#define STORM_STATIC_ASSERT(Condition, Msg) static_assert(Condition, "COMPILE ERROR : " Msg)
#define STORM_STATIC_ASSERT_TO_VS_OUTPUT(Condition, Msg) STORM_STATIC_ASSERT(Condition, Msg ". For more details, see VS Outputs.")
#define STORM_COMPILE_ERROR(Msg) STORM_STATIC_ASSERT_TO_VS_OUTPUT(false, Msg)
