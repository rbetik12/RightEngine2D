#pragma once
#include <string>
#include <unordered_map>
namespace RightEngine
{
    struct ShaderProgramSource
    {
        std::string vertexSource;
        std::string fragmentSource;
    };

    class Shader
    {
    public:
        Shader(const std::string &vertexShaderPath, const std::string &fragmentShaderPath);

        ~Shader();

        void Bind() const;

        void UnBind() const;

        void SetUniform1ui(const std::string &name, uint32_t value);

        void SetUniform1i(const std::string &name, int value);

        void SetUniform1f(const std::string &name, float value);

        void SetUniform2f(const std::string &name, float v0, float v1);

        void SetUniform3f(const std::string &name, float v0, float v1, float v2);

        void SetUniform4f(const std::string &name, float v0, float v1, float v2, float v3);

    private:
        std::string filePath;
        uint32_t id;
        std::unordered_map<std::string, int> uniformLocationCache;

    private:
        ShaderProgramSource ParseShaders(const std::string &vertexShaderPath, const std::string &fragmentShaderPath);

        uint32_t CreateShader(const std::string &vertexShader, const std::string &fragmentShader);

        uint32_t CompileShader(uint32_t type, const std::string &source);

        int GetUniformLocation(const std::string &name);
    };
}