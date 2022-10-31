## Campo Minado

[Link](https://brunoborghese.github.io/abcg/minefield/)

Atividade de avaliação 1: Aplicação interativa com interface do Dear ImGui. Jogo de Campo Minado.

Constantes utilizadas:
- m_N representa o tamanho do quadro.
- m_NBombs indica quantos quadrados conterão bombas.
- m_gameState possui os três estados de jogo possíveis: Jogando(Play), Ganhou(Win), e Perdeu(Lose).
- movesLeft é preenchido com a quantidade de jogadas restantes até vencer, calculado por (m_N * m_N) - m_NBombs.
- m_board representa o estado do quadro, onde cada quadrado é representado por um caracter vazio no início do jogo, e é preenchido quando clicando, com 'X' caso seja uma bomba, ou com a quantidade de bombas na vizinhança imediata.
- m_bombs é preenchido com espaços no ínicio do jogo, e então somente as m_NBombs posições aleatórias são preenchidas com 'b'.

Interface:
- Janela com menu, sem opção de alterar o tamanho, nem colapsar.
- Menu único "Game" com item de reiniciar "Restart".
- Barra de texto centralizada, exibindo de acordo com m_gameState, seguida por espaçamento.
- Tabela composta de botões que representam cada unidade de área do quadro, dividindo igualmente o espaço da tabela.
- Cada botão é inicialmente cinza; após clicado é vermelho caso seja bomba, ou verde caso não; todos com leve incremento nas cores ao passar o mouse e clicar.
- Após espaçamento, um grande botão para reinicar o jogo, com "Restart game" centralizado.

Ao clicar num botão, caso esteja "vazio" e o estado do jogo é "Jogando", é verificado se a poosição pertence à uma bomba, e é calculado a quantidade de bombas na vizinhança. Caso seja uma bomba, o botão é preenchido com 'X', e caso contrário, com a quantidade de bombas, como string.
É então verificado se o jogo chegou ao final, analisando se foi clicado em uma bomba para o estado "Perdeu", ou se acabaram as jogadas para o estado "Ganhou".