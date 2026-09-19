#ifndef LOGGERI_H
#define LOGGERI_H

#include "Logger.h"
#include <map>
#include <string>
#include <mutex>

class LoggerI : public virtual POA_T1Logger::Logger {
private:
    // Estrutura para armazenar o último endereço de cada severidade, o std::map serve para mapear o enum para a string do endereço
    std::map<T1Logger::Severidade, std::string> ultimosEnderecos;
    
    // Mutex para garantir segurança quando diversos clientes chamam o log ao mesmo tempo
    std::mutex mtx;

public:
    LoggerI();
    ~LoggerI();

    virtual void log(
        T1Logger::Severidade severidade,
        const char* endereco,
        CORBA::UShort pid,
        CORBA::Long hora,
        const char* msg
    );

    //  Método locate
    virtual char* locate(T1Logger::Severidade severidade);
};

#endif