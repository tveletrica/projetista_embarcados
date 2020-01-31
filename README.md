MOBIT - Teste de nível para Projetista de Sistemas Embarcados
==============================================================

# Visão do projeto
O teste consiste em desafios para avaliar o nível de conhecimento do candidato em linguagens C e C++, desenvolvimento em Linux, programação paralela, comunicação interprocessos e conhecimentos de sistemas de computação.

Tecnologias:
*   Linux Ubuntu 18.04 ou compatível
*   Linguagem C (gcc 7.4.0 ou superior)
*	Linguagem C\+\+17 (g++ 7.4.0 ou superior)
*	Controle de versão GIT;

---------
# Iniciando
Projeto disponibilizado em: 
    
    http://pdi.mobitbrasil.com.br:8601/projects/ANP/repos/projetista-sis-embarc

Para realizar o teste, faça o clone do repositório em sua máquina local.  
 
    http://pdi.mobitbrasil.com.br:8601/scm/anp/projetista-sis-embarc.git

Utilize o e-mail jobs@mobitbrasil.com.br para dúvidas e esclarecimentos.


---------
# Tarefas

Os seguintes testes buscam avaliar o nível de conhecimento do candidato nas tecnologias citadas acima. O candidato é livre para implementar as soluções da forma que achar mais adequada. Será avaliada, além da solução em si, a clareza do código e a qualidade da documentação. Caso seja preciso instalar pacotes ou bibliotecas adicionais para compilar ou executar o código enviado, os mesmos devem estar listados explicitamente, bem como as instruções completas para o build e execução da solução.

O uso de testes unitários não é obrigatório, mas é incentivado e será considerado um bônus.

É de escolha do canditado a IDE ou editor de textos preferido para codificação, com tudo, deve ser possível realizar o build completo da solução através de um terminal linux 

### 1. Script monitor de recursos
Criar um script que sendo executado pela crontab do sistema, verifica a porcentagem de utilização de todas as partições (/dev/*), o uso de memória RAM e a temperatura dos cores do sistema e envia um e-mail de alerta para cada variável monitorada que estiver acima dos limites estabelecidos dentro do script. O programa também deve gerar um log* quando o uso estiver acima do normal. 

*Para gerar o log, considere que você não tem acesso ou permissão para configurar o rsyslog, dessa forma use o método de redirecionamento de saídas diretamente para o arquivo de log desejado.

### 2. Sincronizando o uso do log com várias threads
    
O código disponibilizado no diretório Q2 se propõe a inicializar várias threads, cada uma tendo como responsabilidade logar uma mensagem na saída padrão. O código, no entanto possui uma série de limitações. Corrija-as a partir dos requisitos a seguir:
    
    2.1. O código é disponibilizado sem um arquivo Makefile. Crie um Makefile que permita a compilação do código sem erros através de um único comando **make** e a exclusão do binário gerado através de um comando **make clean** 
    
    2.2. O código, ao ser executado, mostra algumas mensagens fora de sincronia, com as threads interrompendo as mensagens umas das outras. Modifique o código de modo que cada thread imprima a mensagem do início ao fim, sem que outra thread interrompa a mensagem no meio.
    
    2.3. O código fornecido  imprime o resultado na saída padrão, modifique-o para que o log seja enviado para o syslog do linux.


### 3. Produtor-Consumidor em C++

Implemente em C++ o seguinte cenário:
    
    1 - O programa conta com duas threads independentes: Produtora e Consumidora
    2 - A thread Produtora deve ler o arquivo Q3/input.xml e enviar seu conteúdo para uma fila, de onde a thread Consumidora deve ler e imprimir na tela apenas o conteúdo que estiver dentro das tags <payload></payload>. É importante que seja usado um mecanismo de sincronização entre as threads para tratar as condições de corrida na região crítica.
    
### 4. Comunicação inter processos
Imagine que precisamos ler imagens de uma câmera em tempo real e realizar um processamento para encontrar e recortar a placa do veículo da imagem. O código que faz a leitura da imagem e o que realiza o processamento **precisam rodar em processos diferentes**. Escreva um software em linguagem C que simula esse cenário, seguindo as instruções a seguir:
    
    1 - A leitura da imagem da câmera pode ser feita a partir de uma câmera real (webcam) ou simulada pela leitura de um arquivo em disco com a imagem, a critério do candidato;
    2 - O processo que recebe a imagem **não precisa fazer processamento nenhum**, apenas escrevê-la em disco em um novo arquivo;
    3 - Defina um protocolo simples para controlar o envio da imagem entre os dois processos;
    4 - O processo que faz a leitura e envio da imagem deve enviar uma mensagem para o processo que recebe, informando o tamanho da imagem a ser enviada
    5 - A imagem deve ser enviada entre os processos através do mecanismo de IPC que o candidato julgar mais adequado. Não será aceito que a imagem seja escrita em disco e o path seja passado entre os processos, por uma questão de requisito temporal

---------
Critérios avaliados
-------------------
1.  Solução -  Projeto funcional e atendimento aos itens especificados.
    
2.  Conhecimento da plataforma - Evidências através do código de domínio da programação em ambientes Linux.

3.  Code Style - Código limpo? Código legível? Fácil entendimento para outros desenvolvedores? 

4.	Uso adequado da ferramenta de controle de versão (Diferencial);

---------
Submetendo o código para avaliação
----------------------------------


Quando finalizado e pronto para envio, gere o(s) arquivo(s) de patch com os códigos desenvolvidos.

	git format-patch origin/master

Envie os arquivos de patch gerados por e-mail ao responsável pela aplicação do teste (jobs@mobitbrasil.com.br). Com o git configurado para envio de e-mail, pode ser feito com:

    git send-email  
