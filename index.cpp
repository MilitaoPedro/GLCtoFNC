#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <string>
#include <algorithm>

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
        void eliminaLambda();
        GLC(const std::string& nomeArquivoGNC);
};
    GLC::GLC(const std::string& nomeArquivoGNC){
        glcArquivo.open(nomeArquivoGNC);

        if(glcArquivo.fail()){
            /* Começar execução */
            std::cout << "Arquivo Entrada acessado";
            glcArquivo.close();
            return;
        }

        lerGramatica();
        removeRecursividadeS();
        eliminaLambda();

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
                if (regra.find("S") && linhaVariavel.first != "S") {
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
    std::string nomeArquivoSaida;
    std::cin >> nomeArquivoSaida;
    
    GLC glc(nomeArquivoEntrada);

    return 0;
}