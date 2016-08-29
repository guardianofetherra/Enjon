#ifndef ENJON_GLSLPROGRAM_H
#define ENJON_GLSLPROGRAM_H

#include <GLEW/glew.h>
#include <Math/Maths.h>

#include <string>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Enjon { namespace Graphics {

	class GLSLProgram
	{
	public:
		GLSLProgram();
		~GLSLProgram(); 

		void CreateShader(const char* vertexShaderFilePath, const char* fragmentShaderFilepath); 
		void AddAttribute(const GLchar* attributeName); 
		GLint GetUniformLocation(std::string uniformName); 
		void Use();
		void Unuse();

		void SetUniform1i(std::string name, const int& val);
		void SetUniform1fv(std::string name, float* val, int count);
		void SetUniform1iv(std::string name, int* val, int count);
		void SetUniform1f(std::string name, const float& val);
		void SetUniform2f(std::string name, const Math::Vec2& vector);
		void SetUniform3f(std::string name, const Math::Vec3& vector);
		void SetUniform4f(std::string name, const Math::Vec4& vector);
		void SetUniformMat4(std::string name, const Math::Mat4& matrix); 
		
		GLuint inline GetProgramID() const { return m_programID; } 
	
	private:
		int m_numAttributes; 

		GLuint m_programID;

		GLuint m_vertexShaderID;
		GLuint m_fragmentShaderID;
			
	
	private: 
		void CompileShader(const char* filePath, GLuint id);
		void LinkShaders();
	};

}}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif