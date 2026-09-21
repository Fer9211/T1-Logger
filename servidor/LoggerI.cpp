#include "LoggerI.h"
#include <iostream>
#include <iomanip>
#include <ctime>
#include <cstdio>

LoggerI::LoggerI() : m_verbose(true) {
    // Por padrão, verbose inicia ligado para exibir os logs na tela
}

LoggerI::~LoggerI() {
}

// Getter do atributo verbose
CORBA::Boolean LoggerI::verbose() {
    std::lock_guard<std::mutex> lock(mtx);
    return m_verbose;
}

// Setter do atributo verbose
void LoggerI::verbose(CORBA::Boolean val) {
    std::lock_guard<std::mutex> lock(mtx);
    m_verbose = val;
    std::cout << "\n[LOGGER] Atributo verbose alterado para: " 
              << (m_verbose ? "LIGADO (saída na tela ativada)" : "DESLIGADO (modo silencioso)") 
              << std::endl;
}

// Método assíncrono para registro de eventos (notificação assíncrona oneway)
void LoggerI::log(
    T1Logger::Severidade severidade,
    const char* endereco,
    CORBA::UShort pid,
    CORBA::Long hora,
    const char* msg
) {
    std::lock_guard<std::mutex> lock(mtx);

    // Registra o endereço mais recente associado à severidade
    std::string strEndereco = (endereco ? endereco : "");
    ultimosEnderecos[severidade] = strEndereco;

    // Se verbose estiver ativado, imprime os dados recebidos na tela
    if (m_verbose) {
        const char* sevStr = "DESCONHECIDO";
        switch (severidade) {
            case T1Logger::DEBUG:
                sevStr = "DEBUG";
                break;
            case T1Logger::WARNING:
                sevStr = "WARNING";
                break;
            case T1Logger::ERROR:
                sevStr = "ERROR";
                break;
            case T1Logger::CRITICAL:
                sevStr = "CRITICAL";
                break;
        }

        // Converte timestamp em data e hora legíveis
        time_t t = static_cast<time_t>(hora);
        struct tm* tm_info = localtime(&t);
        char timeBuffer[64];
        if (tm_info != nullptr) {
            strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", tm_info);
        } else {
            snprintf(timeBuffer, sizeof(timeBuffer), "%ld (timestamp)", static_cast<long>(hora));
        }

        std::cout << "----------------------------------------" << std::endl;
        std::cout << "[LOG RECEBIDO]" << std::endl;
        std::cout << "  Severidade : " << sevStr << std::endl;
        std::cout << "  Endereço   : " << strEndereco << std::endl;
        std::cout << "  PID        : " << pid << std::endl;
        std::cout << "  Data/Hora  : " << timeBuffer << " (" << hora << " s)" << std::endl;
        std::cout << "  Mensagem   : " << (msg ? msg : "") << std::endl;
        std::cout << "----------------------------------------" << std::endl;
    }
}

// Método locate: retorna o endereço do último evento com severidade s
char* LoggerI::locate(T1Logger::Severidade severidade) {
    std::lock_guard<std::mutex> lock(mtx);

    auto it = ultimosEnderecos.find(severidade);
    if (it == ultimosEnderecos.end() || it->second.empty()) {
        const char* sevStr = "DESCONHECIDO";
        switch (severidade) {
            case T1Logger::DEBUG:    sevStr = "DEBUG"; break;
            case T1Logger::WARNING:  sevStr = "WARNING"; break;
            case T1Logger::ERROR:    sevStr = "ERROR"; break;
            case T1Logger::CRITICAL: sevStr = "CRITICAL"; break;
        }

        std::string descErro = std::string("Nenhum evento com severidade '") + sevStr + "' foi recebido ainda.";
        T1Logger::EventNotFound ex;
        ex.mensagem = CORBA::string_dup(descErro.c_str());
        throw ex;
    }

    // No mapeamento CORBA C++, strings retornadas devem ser alocadas com CORBA::string_dup
    return CORBA::string_dup(it->second.c_str());
}
