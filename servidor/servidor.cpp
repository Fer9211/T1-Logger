#include "LoggerI.h"
#include <iostream>
#include <orbsvcs/CosNamingC.h>

int main(int argc, char* argv[]) {
    try {
        // inicializa ORB
        CORBA::ORB_var orb = CORBA::ORB_init(argc, argv);

        // obtem ref p RootPOA e ativa POAManager
        CORBA::Object_var obj = orb->resolve_initial_references("RootPOA");
        PortableServer::POA_var root_poa = PortableServer::POA::_narrow(obj.in());
        
        PortableServer::POAManager_var poa_manager = root_poa->the_POAManager();
        poa_manager->activate();

        // cria instancia do Servant 
        LoggerI* logger_servant = new LoggerI();
        PortableServer::ServantBase_var owner_def(logger_servant);

        // registra servant no RootPOA e pega sua ref CORBA
        PortableServer::ObjectId_var oid = root_poa->activate_object(logger_servant);
        CORBA::Object_var ref = root_poa->id_to_reference(oid.in());
        T1Logger::Logger_var logger_ref = T1Logger::Logger::_narrow(ref.in());

        // publica ref no NameService
        CORBA::Object_var naming_obj = orb->resolve_initial_references("NameService");
        CosNaming::NamingContext_var naming_context = CosNaming::NamingContext::_narrow(naming_obj.in());

        if (CORBA::is_nil(naming_context.in())) {
            std::cerr << "Erro: Servidor de Nomes (NameService) nao disponivel!" << std::endl;
            return 1;
        }

        // define nome p servico no NameService
        CosNaming::Name name;
        name.length(1);
        name[0].id = CORBA::string_dup("LoggerService");
        name[0].kind = CORBA::string_dup("");

        // registra servico
        naming_context->rebind(name, logger_ref.in());

        std::cout << "Servidor Logger iniciado com sucesso e registrado no NameService." << std::endl;
        std::cout << "Aguardando chamadas dos clientes..." << std::endl;

        // loop principal do ORB p aguardar chamadas remotas
        orb->run();

        // limpa limpa
        orb->destroy();

    } catch (const CORBA::Exception& e) {
        std::cerr << "Excecao CORBA no servidor: " << e << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Excecao desconhecida no servidor." << std::endl;
        return 1;
    }

    return 0;
}