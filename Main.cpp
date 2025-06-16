#include "EdubotLib.hpp"
#include <iostream>

// Suas constantes, sem alterações.
const double FORWARD_SPEED = 0.4;
const double CRITICAL_FRONT_DISTANCE = 0.15;
const double WALL_LOST_DISTANCE = 1;
const int LOOP_DELAY_MS = 100;

// Máquina de estados aprimorada
enum RobotState { SEGUINDO_PAREDE, BUSCANDO_NOVA_PAREDE };

int main() {
    EdubotLib edubot;

    if (!edubot.connect()) {
        std::cout << "Nao foi possivel conectar ao robo." << std::endl;
        return -1;
    }

    std::cout << "Conectado ao robo! Iniciando o algoritmo..." << std::endl;
    edubot.sleepMilliseconds(2000);

    RobotState currentState = SEGUINDO_PAREDE;

    while (edubot.isConnected()) {
        // Lógica baseada no estado atual
        switch (currentState) {
            case BUSCANDO_NOVA_PAREDE: {
                std::cout << "[Estado: BUSCANDO] Avancandooo para encontrar a nova parede..." << std::endl;
                edubot.sleepMilliseconds(150);
                // Verifica se um obstáculo apareceu na frente durante a busca
                if(edubot.getSonar(3) < CRITICAL_FRONT_DISTANCE){
                    std::cout << "Obstaculo a frente durante a busca! Retornando ao modo normal." << std::endl;
                    edubot.stop();
                    currentState = SEGUINDO_PAREDE;
                    break; // Sai do switch e deixa a lógica principal lidar com o obstáculo no próximo loop
                }

                // Verifica se a parede à direita foi encontrada
                if (edubot.getSonar(6) < WALL_LOST_DISTANCE) {
                    std::cout << "Nova parede encontrada! Retomando o seguimento." << std::endl;
                    edubot.stop();
                    currentState = SEGUINDO_PAREDE;
                } else {
                    // Se não encontrou, continua avançando
                    edubot.move(FORWARD_SPEED);
                }
                break;
            }

            case SEGUINDO_PAREDE: {
                double front_sonar = edubot.getSonar(3);
                double right_sonar = edubot.getSonar(6);

                std::cout << "[Estado: SEGUINDO] Frontal: " << front_sonar << "m, Direito: " << right_sonar << "m" << std::endl;

                // 1. OBSTÁCULO À FRENTE
                if (front_sonar < CRITICAL_FRONT_DISTANCE) {
                    std::cout << "Obstaculo a frente! Virando a esquerda." << std::endl;
                    edubot.stop();
                    edubot.rotate(-90.0);
                    edubot.sleepMilliseconds(1000);
                    // Não precisa mais do estado de recuperação, a lógica normal já é estável
                }
                // 2. PAREDE À DIREITA PERDIDA (Sua lógica de curva aberta)
                else if (right_sonar > WALL_LOST_DISTANCE) {
                    std::cout << "Parede a direita perdida! Iniciando manobra de curva..." << std::endl;
                    // Atraso para o corpo do robô passar a quina
                    edubot.sleepMilliseconds(600);
                    edubot.stop();
                    edubot.rotate(90.0);
                    edubot.sleepMilliseconds(1000);
                    // Movimento para frente para completar a manobra
                    edubot.move(FORWARD_SPEED);
                    edubot.sleepMilliseconds(750);
                    // **MUDANÇA CRUCIAL: Muda o estado para BUSCAR a nova parede**
                    currentState = BUSCANDO_NOVA_PAREDE;
                }
                // 3. TUDO OK, SEGUE EM FRENTE
                else {
                    std::cout << "Seguindo em frente..." << std::endl;
                    edubot.move(FORWARD_SPEED);
                }
                break;
            }
        }

        edubot.sleepMilliseconds(50);
    }

    std::cout << "Desconectado. Fim do programa." << std::endl;
    edubot.disconnect();

    return 0;
}