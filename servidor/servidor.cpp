#include "LoggerI.h"
#include <iostream>
#include <orbsvcs/CosNamingC.h>

int main(int argc, char* argv[]) {
    try {
        //Inicializa o ORB
        CORBA::ORB_var orb = CORBA::ORB_init(argc, argv);

        //Obtém referência para o RootPOA e ativa o POAManager
        CORBA::Object_var obj = orb->resolve_initial_references("RootPOA");
        PortableServer::POA_var root_poa = PortableServer::POA::_narrow(obj.in());
        
        PortableServer::POAManager_var poa_manager = root_poa->the_POAManager();
        poa_manager->activate();

        //Cria a instância do nosso Servant (LoggerI)
        LoggerI* logger_servant = new LoggerI();
        
        //Garante que o servant será destruído corretamente ao encerrar
        PortableServer::ServantBase_var owner_def(logger_servant);

        //Registra o servant no POA e obtém sua referência
        PortableServer::ObjectId_var oid = root_poa->activate_object(logger_servant);
        CORBA::Object_var ref = root_poa->id_to_reference(oid.in());
        T1Logger::Logger_var logger_ref = T1Logger::Logger::_narrow(ref.in());

        //Publica a IOR usando o Servidor de Nomes
        CORBA::Object_var naming_obj = orb->resolve_initial_references("NameService");
        CosNaming::NamingContext_var naming_context = CosNaming::NamingContext::_narrow(naming_obj.in());

        if (CORBA::is_nil(naming_context.in())) {
            std::cerr << "ERRO: O Servidor de Nomes (NameService) não está disponível!" << std::endl;
            return 1;
        }

        //Define o nome com o qual o Logger será registrado no Naming Service
        CosNaming::Name name;
        name.length(1);
        name[0].id = CORBA::string_dup("LoggerService");
        name[0].kind = CORBA::string_dup("");

        //Registra o serviço
        naming_context->rebind(name, logger_ref.in());

        std::cout << "Servidor Logger iniciado com sucesso e registrado no Naming Service." << std::endl;
        std::cout << "Aguardando requisições dos clientes..." << std::endl;

        //Executa o loop principal do ORB para aguardar chamadas remotas
        orb->run();

        //Limpeza final
        orb->destroy();
    } catch (const CORBA::Exception& e) {
        std::cerr << "Exceção CORBA capturada no servidor: " << e << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Exceção desconhecida capturada no servidor." << std::endl;
        return 1;
    }

    return 0;
}