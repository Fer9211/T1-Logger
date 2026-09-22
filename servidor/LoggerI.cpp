#include "LoggerI.h"
#include <iostream>

LoggerI::LoggerI() {
}

LoggerI::~LoggerI() {
}

// Função auxiliar simples para converter severidade para texto
static const char* severidadeParaTexto(T1Logger::Severidade sev) {
    switch (sev) {
        case T1Logger::DEBUG:    return "DEBUG";
        case T1Logger::WARNING:  return "WARNING";
        case T1Logger::ERROR:    return "ERROR";
        case T1Logger::CRITICAL: return "CRITICAL";
        default:                 return "DESCONHECIDO";
    }
}

// Método assíncrono para registro de eventos (oneway void log)
void LoggerI::log(
    T1Logger::Severidade severidade,
    const char* endereco,
    CORBA::UShort pid,
    CORBA::Long hora,
    const char* msg
) {
    // Garante exclusão mútua caso múltiplos clientes enviem logs ao mesmo tempo
    std::lock_guard<std::mutex> lock(mtx);

    // Salva o último endereço recebido para esta severidade
    ultimosEnderecos[severidade] = (endereco ? endereco : "");

    // Imprime os dados recebidos na tela do servidor a cada chamada remota
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "[LOG RECEBIDO]" << std::endl;
    std::cout << "  Severidade: " << severidadeParaTexto(severidade) << std::endl;
    std::cout << "  Endereço  : " << (endereco ? endereco : "") << std::endl;
    std::cout << "  PID       : " << pid << std::endl;
    std::cout << "  Hora      : " << hora << " (segundos)" << std::endl;
    std::cout << "  Mensagem  : " << (msg ? msg : "") << std::endl;
    std::cout << "----------------------------------------" << std::endl;
}

// Método locate: retorna o endereço do último evento com severidade s
char* LoggerI::locate(T1Logger::Severidade severidade) {
    std::lock_guard<std::mutex> lock(mtx);

    auto it = ultimosEnderecos.find(severidade);
    if (it == ultimosEnderecos.end() || it->second.empty()) {
        // Se ainda não recebeu eventos dessa severidade, lança a exceção EventNotFound
        T1Logger::EventNotFound ex;
        std::string erro = std::string("Nenhum evento recebido com severidade ") + severidadeParaTexto(severidade);
        ex.mensagem = CORBA::string_dup(erro.c_str());
        throw ex;
    }

    // Em CORBA C++, strings retornadas pelo servant devem ser alocadas com CORBA::string_dup
    return CORBA::string_dup(it->second.c_str());
}
