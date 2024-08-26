#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <string>
#include <algorithm>
#include <set> 
#include <cctype>
#include <regex>

class GLC{
    private: 
        // Tabela Hash que associa as variáveis (char) a suas regras (vetor de strings)
        std::unordered_map<std::string, std::vector<std::string>> glcHash;
        // Arquivo de entrada
        std::ifstream glcArquivo;
        // Arquivo de saída
        std::ofstream fncArquivo;

    public: 
        void adicionarRegra
        (
            const std::string& variavel, 
            const std::string& regra
        );
        void lerGramatica();
        std::string removeRecursividadeS();
        void imprimirGramatica();
        void removeLambda(const std::string& inicial);
        void combinacoesSemAnulavel
        (
            const std::string& regra, 
            const std::set<std::string>& nullables, 
            std::set<std::string>& regraAtualizadas,
            const std::string& varAtual,
            const std::string& inicial
        );
        void removeChainRules();
        void term();
        void reach();
        void transformarParaFNC();  // Nova função para transformar para FNC
        void substituirTerminais();
        void dividirRegras();
        void renomeiaTs(const int& qntdTs);
        void imprimirArquivoSaida();

        GLC(const std::string& nomeArquivoGNC, const std::string& nomeArquivoFNC);

};
GLC::GLC(const std::string& nomeArquivoGNC, const std::string& nomeArquivoFNC){
    glcArquivo.open(nomeArquivoGNC);

    if(glcArquivo.fail()){
        std::cout << "Arquivo não encontrado";
        glcArquivo.close();
        return;
    }

    lerGramatica();
    removeLambda(removeRecursividadeS());
    removeChainRules();
    term();
    transformarParaFNC();

    fncArquivo.open(nomeArquivoFNC);

    if(fncArquivo.fail()){
        std::cout << "Ocorreu um erro ao criar o arquivo de saída";
        fncArquivo.close();
        return;
    }

    imprimirArquivoSaida();

    // Fecha os arquivos de entrada e saída
    fncArquivo.close();
    glcArquivo.close();
}

void GLC::adicionarRegra(const std::string& variavel, const std::string& regra){
    std::cout << "Vai ser adicionado: " << variavel << " -> " << regra << '\n';
    glcHash[variavel].push_back(regra);
}

void GLC::lerGramatica(){

    std::string variavel;
    std::string regra;
    std::string linhaGramatica;

    while(glcArquivo >> variavel){
        // Pega a linha com todas as regras de determinada variável
        std::getline(glcArquivo, linhaGramatica);

        std::cout << linhaGramatica << '\n';

        // Apaga todos os espaços.
        linhaGramatica.erase((remove_if(linhaGramatica.begin(), linhaGramatica.end(), isspace)), linhaGramatica.end());

        // Remove "->"
        linhaGramatica.erase(0, 2);

        int posicaoDelimitador;

        // Pega todas as regras separadas por '|'
        while ((posicaoDelimitador = linhaGramatica.find('|')) != std::string::npos) {
            regra = linhaGramatica.substr(0, posicaoDelimitador);
            adicionarRegra(variavel, regra);
            linhaGramatica.erase(0, (posicaoDelimitador + 1));
        }

        // Pega a última regra da variável (Não delimitada por '|')
        if(linhaGramatica != ""){
            regra = linhaGramatica;
            adicionarRegra(variavel, regra);
        }
    }
}

std::string GLC::removeRecursividadeS() {
    bool regraS = false;
    for (const auto& linhaVariavel : glcHash) {
        for (const auto& regra : linhaVariavel.second) {
            if ((regra.find("S") != std::string::npos)) {
                regraS = true;
                break;
            }
        }
        if (regraS) break;
    }

    std::string inicial = "S";

    if (regraS) {
        adicionarRegra("S'", "S");
        inicial = "S'";
    }
    
    return inicial;
}

void GLC::removeLambda(const std::string& inicial) {

    // Set permite apenas elementos únicos
    // Busca O(log n) vs O(n) do Vector
    std::set<std::string> nullables;
    bool nullableMudou;

    // Inicializa o conjunto NULL com variáveis que têm regra lambda
    for (const auto& linhaVariavel : glcHash) {
        for (const auto& regra : linhaVariavel.second) {
            if (regra == ".") {
                nullables.insert(linhaVariavel.first);
                break;
            }
        }
    }

    bool todosAnulaveis;

    do {
        nullableMudou = false;
        for (const auto& linhaVariavel : glcHash) {
            // Checa se a variável em questão pertence a nullable
            if (nullables.find(linhaVariavel.first) == nullables.end()) {
                for (const auto& regra : linhaVariavel.second) {
                    todosAnulaveis = true;
                    // Percorre cada letra da Regra
                    for (char letra : regra) {
                        std::string stringLetra(1, letra);
                        // Verifica se algum componente da regra é não anulável
                        if (nullables.find(stringLetra) == nullables.end()) {
                            todosAnulaveis = false;
                            break;
                        }
                    }
                    if (todosAnulaveis) {
                        nullables.insert(linhaVariavel.first);
                        nullableMudou = true;
                        break;
                    }
                }
            }
        }
    } while (nullableMudou);

    std::cout << "Anulaveis: {";
    for(const auto& r : nullables){
        std::cout << r << ", ";
    }
    std::cout << "}"; 

    int anulaveisCount = 0;

    // Remover regras lambda e atualizar produções
    for (auto& linhaVariavel : glcHash) {
        std::set<std::string> regraAtualizadasSet;
        for (const auto& regra : linhaVariavel.second) {
            combinacoesSemAnulavel(regra, nullables, regraAtualizadasSet, linhaVariavel.first, inicial);
        }
        // Converter o conjunto de volta para um vetor
        linhaVariavel.second.assign(regraAtualizadasSet.begin(), regraAtualizadasSet.end());
    }
    std::cout << "\n\nGramatica apos remove Lambda: \n";
    imprimirGramatica();
    std::cout << "\n";
}

void GLC::combinacoesSemAnulavel(const std::string& regra, 
                                const std::set<std::string>& nullables, 
                                std::set<std::string>& regraAtualizadas, 
                                const std::string& varAtual,
                                const std::string& inicial
                                ) {
    size_t n = regra.size();
    std::set<std::string> novasRegras;

    if (regra == "." && varAtual != inicial) 
        return;

    // Comece com a regra original
    novasRegras.insert(regra);

    // Para cada posição na regra, verifique se é anulável e crie novas regras removendo-a
    for (size_t i = 0; i < n; i++) {
        std::string simbolo(1, regra[i]);
        // Verificando se o símbolo da regra naposição i pode ser encontrado em nullables
        if (nullables.find(simbolo) != nullables.end()) {
            std::set<std::string> tempRegras;
            for (const auto& r : novasRegras) {
                // Certifique-se de que estamos dentro dos limites da string
                std::string novaRegra = r.substr(0, i);
                if (i + 1 < r.size()) {
                    novaRegra += r.substr(i + 1);
                }

                if ((r.empty() || r == ".") && varAtual == inicial) {
                    novasRegras.insert("."); // Adiciona a regra vazia representada por "." apenas se for a variável inicial
                }
                tempRegras.insert(novaRegra);
            }
            // Adiciona as regras temporárias a novasRegras
            novasRegras.insert(tempRegras.begin(), tempRegras.end());
        }
    }

    // Adicione todas as novas regras geradas ao conjunto para eliminar duplicações
    for (const auto& regra : novasRegras) {
        if(varAtual == inicial && regra.empty())
            regraAtualizadas.insert(".");
        else if (!regra.empty()) 
            regraAtualizadas.insert(regra);
    }

    // Imprime todas as regras geradas
    for (const auto& r : regraAtualizadas) {
        std::cout << " -> " << r;
    }
}

void GLC::removeChainRules() {
    for (auto& linhaVariavel : glcHash) {
        const std::string& variavel = linhaVariavel.first;
        std::set<std::string> visitados;
        std::set<std::string> novasRegras; // Usa set para evitar duplicações

        std::vector<std::string> fila;
        fila.push_back(variavel);

        while (!fila.empty()) {
            std::string regra = fila.back();
            fila.pop_back();

            if (visitados.find(regra) != visitados.end()) {
                continue;
            }
            visitados.insert(regra);

            for (const auto& regra : glcHash[regra]) {
                if (regra.size() == 1 && isupper(regra[0])) {
                    fila.push_back(regra);
                } else if(regra.empty() && linhaVariavel.first == "S"){ // Não adicionar regras vazias ou lambda
                    novasRegras.insert(".");
                } else if (!regra.empty()) {
                    novasRegras.insert(regra);
                }
            }
        }

        // Atualizar as regras da variável com as novas regras sem duplicatas
        linhaVariavel.second.assign(novasRegras.begin(), novasRegras.end());
    }
    std::cout << "\n\nGramatica apos remove chain Rules: \n";
    imprimirGramatica();
    std::cout << "\n";
}

void GLC::term() {
    std::set<std::string> uteis;
    bool mudou;

    do {
        mudou = false;
        for (const auto& linhaVariavel : glcHash) {
            const std::string& variavel = linhaVariavel.first;
            for (const auto& regra : linhaVariavel.second) {
                bool regraGeradora = true;
                for (char simbolo : regra) {
                    std::string stringSimbolo(1, simbolo);
                    if (!islower(simbolo) && uteis.find(stringSimbolo) == uteis.end()) {
                        regraGeradora = false;
                        break;
                    }
                }
                if (regraGeradora && uteis.find(variavel) == uteis.end()) {
                    uteis.insert(variavel);
                    mudou = true;
                }
            }
        }
    } while (mudou);

    for (auto it = glcHash.begin(); it != glcHash.end(); ) {
        if (uteis.find(it->first) == uteis.end()) {
            it = glcHash.erase(it);
        } else {
            auto& regras = it->second;
            regras.erase(std::remove_if(regras.begin(), regras.end(), [&uteis](const std::string& regra) {
                return std::any_of(regra.begin(), regra.end(), [&uteis](char simbolo) {
                    return isupper(simbolo) && uteis.find(std::string(1, simbolo)) == uteis.end();
                });
            }), regras.end());
            it++;
        }
    }
    
    std::cout << "\n\nGramatica apos Term: \n";
    imprimirGramatica();
    std::cout << "\n";

    reach();
}

void GLC::reach() {
    std::set<std::string> acessiveis;
    std::vector<std::string> fila = {"S"};

    // Checa se existe S' e adiciona-o aos acessiveis
    if(glcHash.find("S'") != glcHash.end())
        acessiveis.insert("S'");

    acessiveis.insert("S");

    while (!fila.empty()) {
        std::string regra = fila.back();
        fila.pop_back();

        for (const auto& regra : glcHash[regra]) {
            for (char simbolo : regra) {
                std::string stringSimbolo(1, simbolo);
                if (isupper(simbolo) && acessiveis.find(stringSimbolo) == acessiveis.end()) {
                    acessiveis.insert(stringSimbolo);
                    fila.push_back(stringSimbolo);
                }
            }
        }
    }

    for (auto it = glcHash.begin(); it != glcHash.end(); ) {
        if (acessiveis.find(it->first) == acessiveis.end()) {
            it = glcHash.erase(it);
        } else {
            ++it;
        }
    }
    std::cout << "\n\nGramatica apos Reach: \n";
    imprimirGramatica();
    std::cout << "\n";
}

void GLC::transformarParaFNC() {
    substituirTerminais();
    imprimirGramatica();  // Passo 1: Substituir terminais
    // dividirRegrasLongas();
    std::cout << "\n\nPASSO 2: "; // Passo 2: Dividir regras longas
    dividirRegras();
    imprimirGramatica(); 
}

void GLC::substituirTerminais() {

    std::unordered_map<char, std::string> TerminalParaNaoTerminal;  // Mapeia terminais para novos não-terminais

    for (auto& linhaVariavel : glcHash) {
        std::vector<std::string>& regras = linhaVariavel.second;
        for (auto& regra : regras) {
            std::string novaRegra;
            if(regra.size() > 1){
                std::cout << "\nEntrou no looping, Regra: " << regra << "\n\n";
                for (size_t i = 0; i < regra.size(); i++) {
                    if (islower(regra[i])) {  // Verificar se é terminal
                        char terminal = regra[i];
                        std::cout << "Terminal / Regra[i] " << terminal << "\n\n";

                        // Procura se já existe um não terminal para esse terminal
                        if (TerminalParaNaoTerminal.find(terminal) == TerminalParaNaoTerminal.end()) {
                            std::cout << "Não encontrou o terminal na lista " << "\n\n";
                            // Criar um novo não-terminal para este terminal
                            char terminalUppercase = regra[i];
                            std::cout << "terminalUppercase {" << terminalUppercase << "}\n\n";
                            terminalUppercase = toupper(terminalUppercase);
                            std::cout << "terminalUppercase apos toupper {" << terminalUppercase << "}\n\n";

                            std::string novoSimbolo = std::string(1, terminalUppercase) + "'";

                            TerminalParaNaoTerminal[terminal] = novoSimbolo;
                            std::cout << "NovoSimbolo {" << novoSimbolo << "}\n\n";
                            glcHash[novoSimbolo].push_back(std::string(1, terminal));  // Adicionar regra novoSimbolo -> terminal
                        }
                        // Substituir o terminal pelo novo não-terminal na regra
                        novaRegra += TerminalParaNaoTerminal[terminal];
                    } else {
                        novaRegra += regra[i];  // Manter não-terminais
                    }
                }
                regra = novaRegra; // Atualizar a regra com a nova regra construída
            }
        }
    }
}

void GLC::dividirRegras() {
    std::unordered_map<std::string, std::vector<std::string>> novasRegras;  // Para armazenar as novas regras geradas
    int contadorTemporario = 1;  // Contador para criar nomes únicos para variáveis temporárias

    std::unordered_map<std::string, std::string> novasRegrasT; // Regras Temporárias

    for (auto& linhaVariavel : glcHash) {
        const std::string& variavel = linhaVariavel.first;
        const std::vector<std::string>& regras = linhaVariavel.second;

        for (const std::string& regra : regras) {
            if (regra.length() > 2) {
                std::string atualLHS = variavel;
                std::vector<std::string> simbolos;

                // Separar a regra em símbolos, considerando apostrofos
                for (size_t i = 0; i < regra.length(); i++) {
                    if (i + 1 < regra.length() && regra[i + 1] == '\'') {
                        simbolos.push_back(regra.substr(i, 2));
                        i++;  // Pular o apostrofo
                    } else {
                        simbolos.push_back(std::string(1, regra[i]));
                    }
                }
                // Criar novas regras para cada par de símbolos
                while(simbolos.size() > 2){
                    
                    // Pega o último símbolo
                    std::string ultimoSimbolo = simbolos.back();
                    simbolos.pop_back();
                    // Pega o penúltimo símbolo
                    std::string penultimoSimbolo = simbolos.back();
                    simbolos.pop_back();

                    bool jaExiste = false;

                    for (const auto& regraT : novasRegrasT) {
                        if(regraT.second == penultimoSimbolo + ultimoSimbolo)
                            // Adiciona a variável T (já existente) a simbolos
                            simbolos.push_back(regraT.first);
                            jaExiste = true;
                            break;
                    }

                    if(!jaExiste){
                        // Cria a variável Tn para os símbolos retirados
                        std::string novoLHS = "T" + std::to_string(contadorTemporario++);

                        // Cria uma linha com o simbolo Tn e os dois ultimos simbolos
                        novasRegras[novoLHS].push_back(penultimoSimbolo + ultimoSimbolo);

                        // Cria uma linha para as variáveis T
                        novasRegrasT[novoLHS].append(penultimoSimbolo + ultimoSimbolo);

                        simbolos.push_back(novoLHS);
                    }
                }

                std::cout << "\nSimbolos da Regra " << atualLHS << " {";
                for(const auto& s : simbolos){
                    std::cout << s << ", ";
                }
                std::cout << "}\n";

                // Concatenar os símbolos restantes em uma única string
                std::string regraFinal;
                for (const auto& s : simbolos) {
                    regraFinal += s;
                }

                // Muda a regra da variável para a string concatenada
                novasRegras[atualLHS].push_back(regraFinal);

            } else {
                novasRegras[variavel].push_back(regra);
            }
        }
    }
    glcHash = novasRegras;
    
    renomeiaTs((contadorTemporario - 1));  // Atualiza o glcHash com as novas regras divididas
}

void GLC::renomeiaTs(const int& qntdTs){

    std::regex tRegex("T(\\d+)");

    std::unordered_map<std::string, std::vector<std::string>> novasRegras;
    
    for (const auto& linhaVariavel : glcHash) {
        std::string novaVariavel = linhaVariavel.first;
        if (novaVariavel[0] == 'T') {
            int numero = std::stoi(novaVariavel.substr(1));
            novaVariavel = "T" + std::to_string((qntdTs + 1) - numero);
        }
        std::vector<std::string> novasProducoes;
        for (const auto& regra : linhaVariavel.second) {
            std::string novaRegra = regra;
            auto begin = std::sregex_iterator(regra.begin(), regra.end(), tRegex);
            auto end = std::sregex_iterator();
            for (std::sregex_iterator i = begin; i != end; ++i) {
                int numero = std::stoi((*i).str(1));
                std::string novoNome = "T" + std::to_string((qntdTs+1) - numero);
                novaRegra.replace((*i).position(), (*i).length(), novoNome);
            }
            novasProducoes.push_back(novaRegra);
        }
        novasRegras[novaVariavel] = novasProducoes;
    }

    glcHash = novasRegras;
}

void GLC::imprimirGramatica(){
    // Exibe todas as regras da gramática
    for (const auto& linhaVariavel : glcHash) {
        std::cout << linhaVariavel.first << " -> ";
        for (size_t i = 0; i < linhaVariavel.second.size(); i++) {
            std::cout << linhaVariavel.second[i];
            if (i < linhaVariavel.second.size() - 1) {
                std::cout << " | ";
            }
        }
        std::cout << std::endl;
    }
}

void GLC::imprimirArquivoSaida() {
    std::vector<std::string> vetHashOrdenada;

    // Busca S' e S, para colocar na frente do vetor
    if (glcHash.find("S'") != glcHash.end()) vetHashOrdenada.push_back("S'");
    if (glcHash.find("S") != glcHash.end()) vetHashOrdenada.push_back("S");

    // Coloca as variáveis em ordem alfabética
    for (char ch = 'A'; ch <= 'Z'; ch++) {
        std::string letra(1, ch);
        if (glcHash.find(letra) != glcHash.end() && letra != "S") 
            vetHashOrdenada.push_back(letra);
    }

    // Coloca as variáveis ' em ordem alfabética
    for (char ch = 'A'; ch <= 'Z'; ++ch) {
        std::string letra(1, ch);
        letra += "'";
        if (glcHash.find(letra) != glcHash.end() && letra != "S'") 
            vetHashOrdenada.push_back(letra);
    }

    // Coloca os Tn em ordem numérica
    for (int i = 1; ; ++i) {
        std::string temps = "T" + std::to_string(i);
        if (glcHash.find(temps) != glcHash.end()) {
            vetHashOrdenada.push_back(temps);
        } else {
            break;  // Para quando não houver mais chaves no formato "Tn"
        }
    }

    // Imprime o vetor vetHashOrdenada no arquivo de saída
    for (const auto& variavel : vetHashOrdenada) {
        fncArquivo << variavel << " -> ";
        const auto& regras = glcHash[variavel];
        for (size_t i = 0; i < regras.size(); ++i) {
            fncArquivo << regras[i];
            if (i < regras.size() - 1) {
                fncArquivo << " | ";
            }
        }
        fncArquivo << '\n';
    }
}

int main(){

    // Pega o arquivo de entrada
    std::string nomeArquivoEntrada;
    std::cin >> nomeArquivoEntrada;

    // Pega o arquivo de saída
    std::string nomeArquivoSaida;
    std::cin >> nomeArquivoSaida;
    
    GLC glc(nomeArquivoEntrada, nomeArquivoSaida);

    return 0;
}
