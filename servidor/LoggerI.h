#ifndef LOGGERI_H
#define LOGGERI_H

#if defined(__has_include)
  #if __has_include("LoggerS.h")
    #include "LoggerS.h"
  #elif __has_include("Logger.h")
    #include "Logger.h"
  #endif
#else
  #include "LoggerS.h"
#endif

#include <map>
#include <string>
#include <mutex>

class LoggerI : public virtual POA_T1Logger::Logger {
private:
    // Estrutura para armazenar o último endereço de cada severidade
    std::map<T1Logger::Severidade, std::string> ultimosEnderecos;

    // Atributo verbose para acionar/desligar a saída na tela
    CORBA::Boolean m_verbose;

    // Mutex para garantir segurança com múltiplos clientes simultâneos
    std::mutex mtx;

public:
    LoggerI();
    virtual ~LoggerI();

    // Métodos de acesso para o atributo verbose (conforme IDL)
    virtual CORBA::Boolean verbose();
    virtual void verbose(CORBA::Boolean val);

    // Método assíncrono para registrar eventos de log
    virtual void log(
        T1Logger::Severidade severidade,
        const char* endereco,
        CORBA::UShort pid,
        CORBA::Long hora,
        const char* msg
    );

    // Método locate para buscar o último endereço da severidade
    virtual char* locate(T1Logger::Severidade severidade);
};

#endif // LOGGERI_H