MOBIT - Teste de nível para Projetista de Sistemas Embarcados
==============================================================

Visão do projeto
----------------
O teste consiste em desafios para avaliar o nível de conhecimento do candidato em linguagens C e C++, desenvolvimento em Linux, programação paralela e conhecimentos de sistemas de computação.

Tecnologias:
*   Linux Ubuntu 18.04 ou compatível
*   Linguagem C (gcc 7.4.0 ou superior)
*	Linguagem C\+\+17 (g++ 7.4.0 ou superior)
*	Controle de versão GIT;


Iniciando
---------
Projeto disponibilizado em: 
    
    http://pdi.mobitbrasil.com.br:8601/projects/ANP/repos/projetista-sis-embarc

Para realizar o teste, faça o clone do repositório em sua máquina local.  
 
    http://pdi.mobitbrasil.com.br:8601/scm/anp/projetista-sis-embarc.git

Utilize o e-mail jobs@mobitbrasil.com.br para dúvidas e esclarecimentos.

Tarefas
-------
Os seguintes testes buscam avaliar o nível de conhecimento do candidato nas tecnologias citadas acima. O candidato é livre para implementar as soluções da forma que achar mais adequada. Será avaliada, além da solução em si, a clareza do código e a qualidade da documentação. Caso seja preciso instalar pacotes ou bibliotecas adicionais para compilar ou executar o código enviado, os mesmos devem estar listados explicitamente, bem como as instruções completas para o build e execução da solução.

O uso de testes unitários não é obrigatório, mas é incentivado e será considerado um bônus.

É de escolha do canditado a IDE ou editor de textos preferido para codificação, com tudo, deve ser possível realizar o build completo da solução através de um terminal linux 











Critérios avaliados
-------------------
1.  Solução        
    *  Projeto funcional e atendimento aos itens especificados.
    
2.  Conhecimento da plataforma
    *   Evidências através do código de domínio da programação em ambientes Linux.

3.  Code Style
    *   Código limpo? Código legível? Fácil entendimento para outros desenvolvedores? 

4.	Uso adequado da ferramenta de controle de versão (Diferencial);

Submetendo o código para avaliação
----------------------------------


Quando finalizado e pronto para envio, gere o(s) arquivo(s) de patch com os códigos desenvolvidos.

	git format-patch origin/master

Envie os arquivos de patch gerados por e-mail ao responsável pela aplicação do teste (jobs@mobitbrasil.com.br). Com o git configurado para envio de e-mail, pode ser feito com:

    git send-email  
