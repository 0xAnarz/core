/*
 *	WebAssembly Loader Tests by Parra Studios
 *
 *	Copyright (C) 2016 - 2021 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	Licensed under the Apache License, Version 2.0 (the "License");
 *	you may not use this file except in compliance with the License.
 *	You may obtain a copy of the License at
 *
 *		http://www.apache.org/licenses/LICENSE-2.0
 *
 *	Unless required by applicable law or agreed to in writing, software
 *	distributed under the License is distributed on an "AS IS" BASIS,
 *	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *	See the License for the specific language governing permissions and
 *	limitations under the License.
 *
 */

#include <gtest/gtest.h>

#include <metacall/metacall.h>

class wasm_loader_test : public testing::Test
{
protected:
	void SetUp() override
	{
		metacall_initialize();
	}

	void TearDown() override
	{
		metacall_destroy();
	}
};

void TestFunction(void *handle, const char *name, const std::vector<enum metacall_value_id> &expected_param_types, enum metacall_value_id expected_return_type)
{
	void *func = metacall_handle_function(handle, name);

	// GoogleTest does not support `ASSERT_NE(NULL, actual)`.
	ASSERT_TRUE(func != NULL);
	ASSERT_EQ(expected_param_types.size(), metacall_function_size(func));

	for (size_t i = 0; i < expected_param_types.size(); i++)
	{
		enum metacall_value_id param_type;
		ASSERT_EQ(0, metacall_function_parameter_type(func, i, &param_type));
		ASSERT_EQ(expected_param_types[i], param_type);
	}

	enum metacall_value_id return_type;
	ASSERT_EQ(0, metacall_function_return_type(func, &return_type));
	ASSERT_EQ(expected_return_type, return_type);
}

TEST_F(wasm_loader_test, InitializeAndDestroy)
{
	// This is extremely hacky and causes an error when loading the buffer,
	// since it is invalid. However, it is currently impossible to initialize a
	// loader without attempting to load a handle with it. Ideally, there would
	// be a `metacall_initialize_loader` function or similar.
	const char dummy_buffer[1] = { 0 };
	metacall_load_from_memory("wasm", dummy_buffer, 1, NULL);

	ASSERT_EQ(0, metacall_is_initialized("wasm"));

	// `metacall_destroy` does nothing if Metacall is not initialized, so we
	// can safely call it here even though we also call it during tear-down.
	ASSERT_EQ(0, metacall_destroy());
}

TEST_F(wasm_loader_test, LoadFromMemory)
{
	// See https://webassembly.github.io/spec/core/binary/modules.html#binary-magic
	const char empty_module[] = {
		0x00, 0x61, 0x73, 0x6d, // Magic bytes
		0x01, 0x00, 0x00, 0x00	// Version
	};
	ASSERT_EQ(0, metacall_load_from_memory("wasm", empty_module, sizeof(empty_module), NULL));

	const char invalid_module[] = { 0 };
	ASSERT_NE(0, metacall_load_from_memory("wasm", invalid_module, sizeof(invalid_module), NULL));
}

// TODO: Make this conditional
//#if defined(OPTION_BUILD_SCRIPTS) && defined(OPTION_BUILD_SCRIPTS_WASM)
TEST_F(wasm_loader_test, LoadFromFile)
{
	const char *empty_module_filename = "empty_module.wasm";
	const char *invalid_module_filename = "invalid_module.wasm";

	ASSERT_EQ(0, metacall_load_from_file("wasm", &empty_module_filename, 1, NULL));
	ASSERT_NE(0, metacall_load_from_file("wasm", &invalid_module_filename, 1, NULL));
}

TEST_F(wasm_loader_test, DiscoverFunctions)
{
	const char *functions_module_filename = "functions.wasm";
	void *handle;

	ASSERT_EQ(0, metacall_load_from_file("wasm", &functions_module_filename, 1, &handle));

	ASSERT_EQ(NULL, metacall_handle_function(handle, "does_not_exist"));

	TestFunction(handle, "none_ret_none", {}, METACALL_INVALID);
	TestFunction(handle, "i32_ret_none", { METACALL_INT }, METACALL_INVALID);
	TestFunction(handle, "i32_f32_i64_f64_ret_none", { METACALL_INT, METACALL_FLOAT, METACALL_LONG, METACALL_DOUBLE }, METACALL_INVALID);
	TestFunction(handle, "none_ret_i32", {}, METACALL_INT);

	// TODO: Add support for multiple return values
	TestFunction(handle, "none_ret_i32_f32_i64_f64", {}, METACALL_INT);
	TestFunction(handle, "i32_f32_i64_f64_ret_i32_f32_i64_f64", { METACALL_INT, METACALL_FLOAT, METACALL_LONG, METACALL_DOUBLE }, METACALL_INT);
}
//#endif
