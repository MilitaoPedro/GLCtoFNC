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
        void adicionarRegra(const std::string& variavel, const std::string& regra);
        void lerGramatica();
        void removeRecursividadeS();
        void imprimirGramatica();
        void removeLambda();
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
        removeRecursividadeS();
        removeLambda();

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

        // Remover regras lambda e atualizar produções
        for (auto& entry : glcHash) {
            std::vector<std::string> updatedRules;
            for (const auto& rule : entry.second) {
                if (rule != ".") {
                    updatedRules.push_back(rule);
                }
                for (size_t i = 0; i < rule.size(); ++i) {
                    if (nullables.find(std::string(1, rule[i])) != nullables.end()) {
                        std::string newRule = rule.substr(0, i) + rule.substr(i + 1);
                        if (!newRule.empty() && std::find(updatedRules.begin(), updatedRules.end(), newRule) == updatedRules.end()) {
                            updatedRules.push_back(newRule);
                        }
                    }
                }
            }
            entry.second = updatedRules;
        }
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
