#include "Shader.hpp"

#include "Utility/File.hpp"
#include "Utility/Config.hpp"

#include <glad/glad.h>
#include "glm/gtc/type_ptr.hpp"

#include <sstream>
#include <stack>
#include <unordered_set>

namespace OpenGL
{
	Shader::Shader(const char* p_name, const std::vector<const char*>& defines)
		: m_name{p_name}
		, m_handle{0}
		, m_uniform_blocks{}
		, m_shader_storage_blocks{}
		, m_uniforms{}
		, m_defines{defines}
		, is_compute_shader{false}
	{
		load_from_file(p_name);
	}

	void Shader::load_from_file(const char* p_name)
	{
		{// Load the shader from shader_path
			// #PERF - Can read the file and run the pre-processing in one step instead of read_from_file and then process_code.

			const auto shader_path = Config::GLSL_Shader_Directory / p_name;

			std::optional<GLHandle> vert_shader;
			{
				auto vert_shader_path = shader_path;
				vert_shader_path.replace_extension("vert");
				if (Utility::File::exists(vert_shader_path))
				{
					std::string source = Utility::File::read_from_file(vert_shader_path);
					source             = process_code(source, m_defines);
					vert_shader        = create_shader(ShaderProgramType::Vertex);
					shader_source(*vert_shader, source);
					compile_shader(*vert_shader);
				}
			}

			std::optional<GLHandle> frag_shader;
			{
				auto frag_shader_path = shader_path;
				frag_shader_path.replace_extension("frag");
				if (Utility::File::exists(frag_shader_path))
				{
					frag_shader        = create_shader(ShaderProgramType::Fragment);
					std::string source = Utility::File::read_from_file(frag_shader_path);
					source             = process_code(source, m_defines);
					shader_source(*frag_shader, source);
					compile_shader(*frag_shader);
				}
			}

			std::optional<GLHandle> geom_shader;
			{
				auto geom_shader_path = shader_path;
				geom_shader_path.replace_extension("geom");
				if (Utility::File::exists(geom_shader_path))
				{
					geom_shader        = create_shader(ShaderProgramType::Geometry);
					std::string source = Utility::File::read_from_file(geom_shader_path);
					source             = process_code(source, m_defines);
					shader_source(geom_shader.value(), source);
					compile_shader(geom_shader.value());
				}
			}

			std::optional<GLHandle> compute_shader;
			{
				auto compute_path = shader_path;
				compute_path.replace_extension("comp");

				if (Utility::File::exists(compute_path))
				{
					// Assert the shader does not have any other shader types.
					ASSERT_THROW(!vert_shader && !frag_shader && !geom_shader, "Compute shader '{}' cannot be used with other shader types", p_name);

					is_compute_shader  = true;
					compute_shader     = create_shader(ShaderProgramType::Compute);
					std::string source = Utility::File::read_from_file(compute_path);
					source             = process_code(source, m_defines);
					shader_source(compute_shader.value(), source);
					compile_shader(compute_shader.value());
				}
			}

			{
				m_handle = create_program();

				if (vert_shader)
					attach_shader(m_handle, *vert_shader);
				if (frag_shader)
					attach_shader(m_handle, *frag_shader);
				if (geom_shader)
					attach_shader(m_handle, *geom_shader);
				if (compute_shader)
					attach_shader(m_handle, *compute_shader);

				link_program(m_handle);

				// Delete the shaders after linking as they're no longer needed, they will be flagged for deletion,
				// but will not be deleted until they are no longer attached to any shader program object.
				if (vert_shader)
					delete_shader(*vert_shader);
				if (frag_shader)
					delete_shader(*frag_shader);
				if (geom_shader)
					delete_shader(*geom_shader);
				if (compute_shader)
					delete_shader(*compute_shader);
			}
		}

		{// After we have linked the program we can query the uniform and shader storage blocks.
			{ // Setup loose uniforms (not belonging to the interface blocks or shader storage blocks)
				const GLint uniform_count = get_uniform_count(m_handle);

				for (int uniform_index = 0; uniform_index < uniform_count; ++uniform_index)
				{
					constexpr GLenum properties[1] = {GL_BLOCK_INDEX};
					GLint values[1];
					glGetProgramResourceiv(m_handle, GL_UNIFORM, uniform_index, 1, properties, 1, NULL, values);

					// If the variable is not the member of an interface block, the value is -1.
					if (values[0] == -1)
						m_uniforms.emplace_back(m_handle, uniform_index, Variable::Type::Uniform);
				}
			}
			{ // m_uniform_blocks setup
				const GLint block_count = get_uniform_block_count(m_handle);
				for (int block_index = 0; block_index < block_count; block_index++)
					m_uniform_blocks.emplace_back(m_handle, block_index, InterfaceBlock::Type::UniformBlock);
			}
			{ // m_shader_storage_blocks
				const GLint block_count = get_shader_storage_block_count(m_handle);
				m_shader_storage_blocks.reserve(block_count);
				for (int block_index = 0; block_index < block_count; block_index++)
					m_shader_storage_blocks.emplace_back(m_handle, block_index, InterfaceBlock::Type::ShaderStorageBlock);
			}
		}

		LOG("OpenGL::Shader '{}' loaded given ID: {}", m_name, m_handle);
	}

	void Shader::reload()
	{
		glDeleteProgram(m_handle);
		m_uniform_blocks.clear();
		m_shader_storage_blocks.clear();
		m_uniforms.clear();
		load_from_file(m_name.c_str());
	}

	Variable::Variable(GLHandle p_shader_program, GLuint p_uniform_index, Type p_type)
	    : m_identifier{""}
	    , m_type{ShaderDataType::Unknown}
	    , m_variable_type{p_type}
	    , m_offset{-1}
	    , m_array_size{-1}
	    , m_array_stride{-1}
	    , m_matrix_stride{-1}
	    , m_is_row_major{false}
	    , m_location{-1}
	    , m_top_level_array_size{-1}
	    , m_top_level_array_stride{-1}
	{
		GLenum type_query = [p_type]
		{
			switch (p_type)
			{
				case Type::Uniform:            return GL_UNIFORM;
				case Type::UniformBlock:       return GL_UNIFORM;
				case Type::ShaderStorageBlock: return GL_BUFFER_VARIABLE;
				default: ASSERT(false, "Unknown shader variable Type"); return GL_INVALID_ENUM;
			}
		}();

		// Use OpenGL introspection API to Query the shader program for properties of its Uniform resources.
		// https://www.khronos.org/opengl/wiki/Program_Introspection
		static constexpr GLenum property_query[7] = {GL_NAME_LENGTH, GL_TYPE, GL_OFFSET, GL_ARRAY_SIZE, GL_ARRAY_STRIDE, GL_MATRIX_STRIDE, GL_IS_ROW_MAJOR};
		GLint property_values[7] = {-1};
		glGetProgramResourceiv(p_shader_program, type_query, p_uniform_index, 7, &property_query[0], 7, NULL, &property_values[0]);

		m_identifier.resize(property_values[0]);
		glGetProgramResourceName(p_shader_program, type_query, p_uniform_index, property_values[0], NULL, m_identifier.data());
		ASSERT(!m_identifier.empty(), "Failed to get name of the interface block variable in shader with handle {}", p_shader_program);
		m_identifier.pop_back(); // glGetProgramResourceName appends the null terminator remove it here.

		m_type          = convert(property_values[1]);
		m_offset        = property_values[2];
		m_array_size    = property_values[3];
		m_array_stride  = property_values[4];
		m_matrix_stride = property_values[5];
		m_is_row_major  = property_values[6];

		if (type_query == GL_UNIFORM) // GL_LOCATION is only valid for Uniforms
		{
			GLenum location_query = GL_LOCATION;
			glGetProgramResourceiv(p_shader_program, GL_UNIFORM, p_uniform_index, 1, &location_query, 1, NULL, &m_location);
		}
		else if (type_query == GL_BUFFER_VARIABLE) // GL_TOP_LEVEL_ARRAY_SIZE and GL_TOP_LEVEL_ARRAY_STRIDE are only valid for GL_BUFFER_VARIABLE
		{
			GLenum buffer_var_query[2] = {GL_TOP_LEVEL_ARRAY_SIZE, GL_TOP_LEVEL_ARRAY_STRIDE};
			GLint buffer_var_vals[2] = {-1};
			glGetProgramResourceiv(p_shader_program, GL_BUFFER_VARIABLE, p_uniform_index, 2, &buffer_var_query[0], 2, NULL, &buffer_var_vals[0]);

			m_top_level_array_size   = buffer_var_vals[0];
			m_top_level_array_stride = buffer_var_vals[1];
		}
	}
	InterfaceBlock::InterfaceBlock(GLHandle p_shader_program, GLuint p_block_index, Type p_type)
	: m_identifier{""}
	, m_variables{}
	, m_block_index{p_block_index}
	, m_type{p_type}
	, m_data_size{-1}
	, m_binding_point{0}
	{
		GLenum block_type_query = [p_type]
		{
			switch (p_type)
			{
				case Type::UniformBlock:       return GL_UNIFORM_BLOCK;
				case Type::ShaderStorageBlock: return GL_SHADER_STORAGE_BLOCK;
				default: ASSERT(false, "Unknown interface block Type"); return GL_INVALID_ENUM;
			}
		}();

		static constexpr size_t Property_Count = 4;
		static constexpr GLenum property_query[Property_Count] = {GL_NAME_LENGTH, GL_NUM_ACTIVE_VARIABLES, GL_BUFFER_BINDING, GL_BUFFER_DATA_SIZE};
		GLint property_values[Property_Count] = {-1};
		glGetProgramResourceiv(p_shader_program, block_type_query, m_block_index, Property_Count, &property_query[0], Property_Count, NULL, &property_values[0]);

		m_identifier.resize(property_values[0]);
		glGetProgramResourceName(p_shader_program, block_type_query, m_block_index, property_values[0], NULL, m_identifier.data());
		ASSERT(!m_identifier.empty(), "Failed to get name of the interface block in shader with handle {}", p_shader_program);
		m_identifier.pop_back(); // glGetProgramResourceName appends the null terminator remove it here.

		m_binding_point = property_values[2];
		m_data_size     = property_values[3];
		const GLint active_variables_count = property_values[1];
		if (active_variables_count > 0)
		{
			// Get the array of active variable indices associated with the the interface block. (GL_ACTIVE_VARIABLES)
			// The indices correspond in size to GL_NUM_ACTIVE_VARIABLES
			std::vector<GLint> variable_indices(active_variables_count);
			static constexpr GLenum active_variable_query[1] = {GL_ACTIVE_VARIABLES};
			glGetProgramResourceiv(p_shader_program, block_type_query, m_block_index, 1, active_variable_query, active_variables_count, NULL, &variable_indices[0]);

			Variable::Type variable_type = [p_type]
			{
				switch (p_type)
				{
					case Type::UniformBlock:       return Variable::Type::UniformBlock;
					case Type::ShaderStorageBlock: return Variable::Type::ShaderStorageBlock;
					default: ASSERT(false, "Unknown interface block Type"); return Variable::Type::UniformBlock;
				}
			}();

			m_variables.reserve(active_variables_count);
			for (GLint variable_index : variable_indices)
				m_variables.emplace_back(p_shader_program, static_cast<GLuint>(variable_index), variable_type);
		}
		ASSERT(m_variables.size() == (size_t)active_variables_count, "Failed to retrieve all the UniformBlockVariables of block '{}'", m_identifier);
	}
	const Variable& InterfaceBlock::get_variable(const char* p_identifier) const
	{
		auto it = std::find_if(m_variables.begin(), m_variables.end(), [p_identifier](const auto& variable)
			{ return variable.m_identifier == p_identifier; });

		ASSERT(it != m_variables.end(), "Variable '{}' not found in InterfaceBlock '{}'", p_identifier, m_identifier);
		return *it;
	}

	void Shader::set_uniform(const char* p_identifier, bool p_value) const
	{
		auto location = get_uniform_variable(p_identifier).m_location;
		glUniform1i(location, (GLint)p_value); // Setting a boolean is treated as integer
	}
	void Shader::set_uniform(const char* p_identifier, int p_value) const
	{
		auto location = get_uniform_variable(p_identifier).m_location;
		glUniform1i(location, (GLint)p_value);
	}
	void Shader::set_uniform(const char* p_identifier, unsigned int p_value) const
	{
		auto location = get_uniform_variable(p_identifier).m_location;
		glUniform1ui(location, (GLuint)p_value);
	}
	void Shader::set_uniform(const char* p_identifier, float p_value) const
	{
		auto location = get_uniform_variable(p_identifier).m_location;
		glUniform1f(location, p_value);
	}
	void Shader::set_uniform(const char* p_identifier, const glm::vec2& p_value) const
	{
		auto location = get_uniform_variable(p_identifier).m_location;
		glUniform2fv(location, 1, glm::value_ptr(p_value));
	}
	void Shader::set_uniform(const char* p_identifier, const glm::vec3& p_value) const
	{
		auto location = get_uniform_variable(p_identifier).m_location;
		glUniform3fv(location, 1, glm::value_ptr(p_value));
	}
	void Shader::set_uniform(const char* p_identifier, const glm::vec4& p_value) const
	{
		auto location = get_uniform_variable(p_identifier).m_location;
		glUniform4fv(location, 1, glm::value_ptr(p_value));
	}
	void Shader::set_uniform(const char* p_identifier, const glm::mat2& p_value) const
	{
		auto location = get_uniform_variable(p_identifier).m_location;
		glUniformMatrix2fv(location, 1, GL_FALSE, glm::value_ptr(p_value));
	}
	void Shader::set_uniform(const char* p_identifier, const glm::mat3& p_value) const
	{
		auto location = get_uniform_variable(p_identifier).m_location;
		glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(p_value));
	}
	void Shader::set_uniform(const char* p_identifier, const glm::mat4& p_value) const
	{
		auto location = get_uniform_variable(p_identifier).m_location;
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(p_value));
	}

	void Shader::bind_sampler_2D(const char* p_identifier, GLuint p_texture_binding)
	{
		GLint index = static_cast<GLint>(p_texture_binding);
		set_uniform(p_identifier, index);
	}
	void Shader::bind_uniform_block(const char* p_identifier, GLuint p_uniform_block_binding)
	{
		auto& block = get_uniform_block(p_identifier);
		if (block.m_binding_point == p_uniform_block_binding)
			return;

		glUniformBlockBinding(m_handle, block.m_block_index, p_uniform_block_binding);
		block.m_binding_point = p_uniform_block_binding;
	}
	void Shader::bind_shader_storage_block(const char* p_identifier, GLuint p_storage_block_binding)
	{
		auto& block = get_shader_storage_block(p_identifier);
		if (block.m_binding_point == p_storage_block_binding)
			return;

		glShaderStorageBlockBinding(m_handle, block.m_block_index, p_storage_block_binding);
		block.m_binding_point = p_storage_block_binding;
	}

	std::string Shader::process_code(const std::string& source_code, const std::vector<const char*>& defined_variables)
	{
		auto is_defined_var = [&defined_variables](const std::string& variable)
		{ return std::find(defined_variables.begin(), defined_variables.end(), variable) != defined_variables.end();};

		std::istringstream code_stream(source_code);
		std::ostringstream result;
		std::string line;

		std::stack<bool> condition_stack;       // Stack to track nesting of conditions
		std::stack<bool> block_processed_stack; // Tracks if any condition in the current block has been true
		bool current_condition = true;

		while (std::getline(code_stream, line))
		{
			std::string trimmed_line = line;
			// trim all whitespace inside the line (spaces, tabs, newlines, etc.)
			trimmed_line.erase(std::remove_if(trimmed_line.begin(), trimmed_line.end(), ::isspace), trimmed_line.end());

			if (trimmed_line.starts_with("#ifdef"))
			{
				std::string variable = trimmed_line.substr(6); // Extract the variable name after #ifdef

				// Determine if this block should be included
				bool is_defined_block = is_defined_var(variable);
				current_condition     = current_condition && is_defined_block;
				condition_stack.push(is_defined_block);
				block_processed_stack.push(is_defined_block);
			}
			else if (trimmed_line.starts_with("#elifdef"))
			{
				std::string variable = trimmed_line.substr(8); // Extract the variable name after #elifdef

				if (!block_processed_stack.empty())
				{
					bool was_processed = block_processed_stack.top();
					if (!was_processed)
					{
						// Only evaluate #elif if no previous condition was true
						bool is_defined_block = is_defined_var(variable);
						current_condition     = current_condition && is_defined_block;
						condition_stack.pop(); // Replace the previous condition with this one
						condition_stack.push(is_defined_block);
						block_processed_stack.pop();
						block_processed_stack.push(is_defined_block);
					}
					else
					{
						// If a previous condition was true, ignore this one
						current_condition = false;
						condition_stack.pop(); // Still need to adjust the stack
						condition_stack.push(false);
					}
				}
			}
			else if (trimmed_line.starts_with("#else"))
			{
				if (!block_processed_stack.empty())
				{
					bool was_processed = block_processed_stack.top();
					condition_stack.pop();
					condition_stack.push(!was_processed);
				}
			}
			else if (trimmed_line.starts_with("#endif"))
			{
				if (!condition_stack.empty())
				{
					condition_stack.pop();
					block_processed_stack.pop();
				}

				// Reset the current_condition based on the stack
				current_condition = condition_stack.empty() ? true : condition_stack.top();
			}
			else
			{
				// Only add lines to the result if they're in an active block
				if (condition_stack.empty() || condition_stack.top())
				{
					result << line << '\n';
				}
			}
		}

		return result.str();
	}

	GLuint Shader::get_attribute_index(const char* attribute_identifier) const
	{
		// queries the previously linked program object specified by program for the attribute variable specified by name and returns the index of the
		// generic vertex attribute that is bound to that attribute variable.
		// If name is a matrix attribute variable, the index of the first column of the matrix is returned.
		// If the named attribute variable is not an active attribute in the specified program a value of -1 is returned.
		GLint index = glGetAttribLocation(m_handle, attribute_identifier);
		ASSERT_THROW(index != -1, "Attribute '{}' not found in shader '{}'", attribute_identifier, m_name);
		return static_cast<GLuint>(index);
	}
	const Variable& Shader::get_uniform_variable(const char* p_identifier) const
	{
		auto it = std::find_if(m_uniforms.begin(), m_uniforms.end(), [p_identifier](const auto& uniform)
			{ return uniform.m_identifier == p_identifier; });

		ASSERT_THROW(it != m_uniforms.end(), "Uniform '{}' not found in shader '{}'", p_identifier, m_name);
		return *it;
	}
	const InterfaceBlock& Shader::get_uniform_block(const char* p_identifier) const
	{
		auto it = std::find_if(m_uniform_blocks.begin(), m_uniform_blocks.end(), [p_identifier](const auto& block)
			{ return block.m_identifier == p_identifier; });

		ASSERT_THROW(it != m_uniform_blocks.end(), "UniformBlock '{}' not found in shader '{}'", p_identifier, m_name);
		return *it;
	}
	InterfaceBlock& Shader::get_uniform_block(const char* p_identifier)
	{
		auto it = std::find_if(m_uniform_blocks.begin(), m_uniform_blocks.end(), [p_identifier](const auto& block)
			{ return block.m_identifier == p_identifier; });

		ASSERT_THROW(it != m_uniform_blocks.end(), "UniformBlock '{}' not found in shader '{}'", p_identifier, m_name);
		return *it;
	}
	const InterfaceBlock& Shader::get_shader_storage_block(const char* p_identifier) const
	{
		auto it = std::find_if(m_shader_storage_blocks.begin(), m_shader_storage_blocks.end(), [p_identifier](const auto& block)
			{ return block.m_identifier == p_identifier; });

		ASSERT_THROW(it != m_shader_storage_blocks.end(), "ShaderStorageBlock '{}' not found in shader '{}'", p_identifier, m_name);
		return *it;
	}
	InterfaceBlock& Shader::get_shader_storage_block(const char* p_identifier)
	{
		auto it = std::find_if(m_shader_storage_blocks.begin(), m_shader_storage_blocks.end(), [p_identifier](const auto& block)
			{ return block.m_identifier == p_identifier; });

		ASSERT_THROW(it != m_shader_storage_blocks.end(), "ShaderStorageBlock '{}' not found in shader '{}'", p_identifier, m_name);
		return *it;
	}
} // namespace OpenGL