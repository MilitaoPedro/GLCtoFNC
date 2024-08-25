#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <string>
#include <algorithm>
#include <set> 

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
        void removeRecursividadeS();
        void imprimirGramatica();
        void removeLambda();
        void combinacoesSemAnulavel
        (
            const std::string& regra, 
            const std::set<std::string>& nullables, 
            std::set<std::string>& regraAtualizadas,
            const std::string& varAtual
        );
        void removeChainRules();
        GLC(const std::string& nomeArquivoGNC);
};
    GLC::GLC(const std::string& nomeArquivoGNC){
        glcArquivo.open(nomeArquivoGNC);

        if(glcArquivo.fail()){
            /* Começar execução */
            std::cout << "Arquivo não encontrado";
            glcArquivo.close();
            return;
        }

        lerGramatica();
        // removeRecursividadeS();
        // removeLambda();
        removeChainRules();
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

    void GLC::removeRecursividadeS() {
        bool regraS = false;
        for (const auto& linhaVariavel : glcHash) {
            for (const auto& regra : linhaVariavel.second) {
                if ((regra.find("S") != std::string::npos) && (linhaVariavel.first != "S")) {
                    std::cout << "Regra S: " << regra << " " << (regra.find("S") == std::string::npos) << '\n';
                    regraS = true;
                    break;
                }
            }
            if (regraS) break;
        }

        if (regraS) {
            adicionarRegra("S'", "S");
        }
    }

    void GLC::removeLambda() {

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
                combinacoesSemAnulavel(regra, nullables, regraAtualizadasSet, linhaVariavel.first);
            }
            // Converter o conjunto de volta para um vetor
            linhaVariavel.second.assign(regraAtualizadasSet.begin(), regraAtualizadasSet.end());
        }

        imprimirGramatica();
    }

    void GLC::combinacoesSemAnulavel(const std::string& regra, const std::set<std::string>& nullables, std::set<std::string>& regraAtualizadas, const std::string& varAtual) {
        size_t n = regra.size();
        std::set<std::string> novasRegras;

        // Se a regra é apenas ".", não adiciona nada
        if (regra == "." && varAtual != "S") {
            return;
        }

        // Comece com a regra original
        novasRegras.insert(regra);

        // Para cada posição na regra, verifique se é anulável e crie novas regras removendo-a
        for (size_t i = 0; i < n; ++i) {
            std::string symbol(1, regra[i]);
            if (nullables.find(symbol) != nullables.end()) {
                std::set<std::string> tempRegras;
                for (const auto& r : novasRegras) {
                    // Certifique-se de que estamos dentro dos limites da string
                    std::string novaRegra = r.substr(0, i);
                    if (i + 1 < r.size()) {
                        novaRegra += r.substr(i + 1);
                    }
                    // Adiciona a nova regra, mesmo se estiver vazia
                    if (r.empty() && varAtual == "S") {
                        novasRegras.insert("."); // Adiciona a regra vazia representada por "." apenas se for a variável inicial
                    }
                    tempRegras.insert(novaRegra);
                }
                // Adiciona as regras temporárias a novasRegras
                novasRegras.insert(tempRegras.begin(), tempRegras.end());
            }
        }

        // Adicione todas as novas regras geradas ao conjunto para eliminar duplicatas
        for (const auto& regra : novasRegras) {
            if(!regra.empty()) 
                regraAtualizadas.insert(regra);
        }

        // Para depuração, imprimir todas as regras geradas
        for (const auto& r : regraAtualizadas) {
            std::cout << " -> " << r;
        }
    }

    void GLC::removeChainRules() {
        for (auto& entry : glcHash) {
            const std::string& variavel = entry.first;
            std::set<std::string> visitados;
            std::set<std::string> novasRegras; // Usar set para evitar duplicatas

            std::vector<std::string> fila;
            fila.push_back(variavel);

            while (!fila.empty()) {
                std::string atual = fila.back();
                fila.pop_back();

                if (visitados.find(atual) != visitados.end()) {
                    continue;
                }
                visitados.insert(atual);

                for (const auto& regra : glcHash[atual]) {
                    if (regra.size() == 1 && isupper(regra[0])) {
                        fila.push_back(regra);
                    } else if(regra.empty() && entry.first == "S"){ // Não adicionar regras vazias ou lambda
                        novasRegras.insert(".");
                    } else if (!regra.empty()) {
                        novasRegras.insert(regra);
                    }
                }
            }

            // Atualizar as regras da variável com as novas regras sem duplicatas
            entry.second.assign(novasRegras.begin(), novasRegras.end());
        }
        imprimirGramatica();
    }

    void GLC::imprimirGramatica(){
         // Exibe todas as regras da gramática
        for (const auto& entry : glcHash) {
            std::cout << entry.first << " -> ";
            for (size_t i = 0; i < entry.second.size(); ++i) {
                std::cout << entry.second[i];
                if (i < entry.second.size() - 1) {
                    std::cout << " | ";
                }
            }
            std::cout << std::endl;
        }
    }

int main(){

    // Pega o arquivo de entrada
    std::string nomeArquivoEntrada;
    std::cin >> nomeArquivoEntrada;

    // Pega o arquivo de saída
    // std::string nomeArquivoSaida;
    // std::cin >> nomeArquivoSaida;
    
    GLC glc(nomeArquivoEntrada);

    return 0;
}
