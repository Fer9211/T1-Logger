#include "LoggerI.h"
#include <iostream>

LoggerI::LoggerI() {
}

LoggerI::~LoggerI() {
}

// parse sev para texto
static const char* severidadeParaTexto(T1Logger::Severidade sev) {
    switch (sev) {
        case T1Logger::DEBUG:    return "DEBUG";
        case T1Logger::WARNING:  return "WARNING";
        case T1Logger::ERROR:    return "ERROR";
        case T1Logger::CRITICAL: return "CRITICAL";
        default:                 return "DESCONHECIDO";
    }
}

// async registro de eventos
void LoggerI::log(
    T1Logger::Severidade severidade,
    const char* endereco,
    CORBA::UShort pid,
    CORBA::Long hora,
    const char* msg
) {
    // garante delete, caso condicao de corrida
    std::lock_guard<std::mutex> lock(mtx);

    // salva o ultimo endereço desse erro
    ultimosEnderecos[severidade] = (endereco ? endereco : "");

    std::cout << "----------------------------------------" << std::endl;
    std::cout << "[LOG RECEBIDO]" << std::endl;
    std::cout << " Severidade: " << severidadeParaTexto(severidade) << std::endl;
    std::cout << "Endereço : " << (endereco ? endereco : "") << std::endl;
    std::cout << "PID: " << pid << std::endl;
    std::cout << "Hora: " << hora << " (segundos)" << std::endl;
    std::cout << "Mensagem: " << (msg ? msg : "") << std::endl;
    std::cout << "----------------------------------------" << std::endl;
}

// locate: retorna o endereco do ultimo evento com severidade s
char* LoggerI::locate(T1Logger::Severidade severidade) {
    std::lock_guard<std::mutex> lock(mtx);

    auto it = ultimosEnderecos.find(severidade);
    if (it == ultimosEnderecos.end() || it->second.empty()) {
        // se não recebeu, lança a exceção
        T1Logger::EventNotFound ex;e
        std::string erro = std::string("Nenhum evento recebido com severidade ") + severidadeParaTexto(severidade);
        ex.mensagem = CORBA::string_dup(erro.c_str());
        throw ex;
    }

    // strings retornadas pelo servant tem q ser alocadas assim
    return CORBA::string_dup(it->second.c_str());
}
