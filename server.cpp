#include <stdio.h>
#include <stdlib.h>
/* You will to add includes here */
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>
#include <unistd.h>

#define MAXCLIENTS 100

using namespace std;

int nrOfPlayers;
int Gamequeue[50];
int qeueu = 0;
int watchQueue[50];
int nrInWatchQueue = 0;
float top[5];
int nrInTop = 0;
string top5;

struct game
{
    int player1;
    int player2;
    int player1Score = 0;
    int player2Score = 0;
    int player1choise = 0;
    int player2choise = 0;
    time_t p1time = 0;
    time_t p2time = 0;
    time_t p1totTime = 0;
    time_t p2totTime = 0;
    bool p1Ready = false;
    bool p2Ready = false;
    int nrOfspectators = 0;
    int spectators[10];
    int rounds = 0;
    int maxRounds = 12;
    bool startRound = false;
    bool p1Set, p2Set;
    bool active = false;
    time_t startTime;
    int count;
    struct itimerval alarmTime;
};

int nrOfGames = 0;
game games[25];

void refreshTop()
{
    printf("REfresing top.\n");
    top5 = "Top 5:\n";
    for (int i = 0; i < nrInTop; i++)
    {
        top5 += to_string(i + 1) + ": " + to_string(top[i]) + "\n";
    }
}
void putInTop(time_t time)
{
    printf("New top????\n");
    if (nrInTop > 5)
    {
        for (int i = 0; i < nrInTop; i++)
        {
            if (time < top[i])
            {
                printf("New top time.\n");
                top[i] = time;
                break;
            }
        }
    }
    else if (nrInTop > 0)
    {
        for (int i = 0; i < nrInTop; i++)
        {
            if (time < top[i])
            {
                printf("New top time.\n");
                top[i] = time;
                break;
            }
        }
        printf("New last.\n");
        top[nrInTop++] = time;
    }
    else if (nrInTop == 0)
    {
        printf("Fist game!!.\n");
        top[nrInTop++] = time;
    }
    refreshTop();
}
string removeWord(string str, string word)
{
    // Check if the word is present in string
    // If found, remove it using removeAll()
    if (str.find(word) != string::npos)
    {
        size_t p = -1;

        // To cover the case
        // if the word is at the
        // beginning of the string
        // or anywhere in the middle
        string tempWord = word + " ";
        while ((p = str.find(word)) != string::npos)
            str.replace(p, tempWord.length(), "");

        // To cover the edge case
        // if the word is at the
        // end of the string
        tempWord = " " + word;
        while ((p = str.find(word)) != string::npos)
            str.replace(p, tempWord.length(), "");
    }

    // Return the resultant string
    return str;
}
int Winner(int p1, int p2)
{
    int win = 0;
    //1: Rock 2: Paper 3: Scissors
    //WIN: 1: p1 winns. 2: p2 winns. 0: Draw
    if (p1 == 1)
    {
        if (p2 == 1)
        {
            //Draw
            win = 0;
        }
        else if (p2 == 2)
        {
            //p2 wins
            win = 2;
        }
        else if (p2 == 3)
        {
            //p1 wins
            win = 1;
        }
        else if (p2 == 0)
        {
            win = 1;
        }
    }
    else if (p1 == 2)
    {
        if (p2 == 1)
        {
            //p1 wins
            win = 1;
        }
        else if (p2 == 2)
        {
            //Draw
            win = 0;
        }
        else if (p2 == 3)
        {
            //p2 wins
            win = 2;
        }
        else if (p2 == 0)
        {
            win = 1;
        }
    }
    else if (p1 == 3)
    {
        if (p2 == 1)
        {
            //p2 wins
            win = 2;
        }
        else if (p2 == 2)
        {
            //p1 wins
            win = 1;
        }
        else if (p2 == 3)
        {
            //Draw
            win = 0;
        }
        else if (p2 == 0)
        {
            win = 1;
        }
    }
    else if (p1 == 0)
    {
        if (p2 == 0)
        {
            //Draw
            win = 0;
        }
        else
        {
            //p2 wins
            win = 2;
        }
    }
    return win;
}
string Menu()
{
    return ("Please select:\n1:Play\n2:Watch\n3:Exit\n4:Top 5\n");
}
void removeFromQeueu(int sock)
{
    for (int i = 0; i < qeueu; i++)
    {
        if (Gamequeue[i] == sock)
        {
            //Found the client
            for (int j = i; j < qeueu; j++)
            {
                Gamequeue[j] = Gamequeue[j + 1];
            }
            qeueu--;
            break;
        }
    }
}
void sendMsg(int sock, string msg)
{
    if (send(sock, msg.c_str(), msg.length(), 0) < 0)
    {
        printf("Sending msg.\n");
    }
}
void endGame(game gameToFind)
{
    for (int i = 0; i < nrOfGames; i++)
    {
        if (games[i].player1 == gameToFind.player1 || games[i].player2 == gameToFind.player2)
        {
            //Found the game
            sendMsg(games[i].player1, Menu());
            sendMsg(games[i].player2, Menu());
            games[i].player1 = 0;
            games[i].player2 = 0;
            games[i].p1Set = false;
            games[i].p2Set = false;
            games[i].active = false;
            for (int j = 0; j < games[i].nrOfspectators; j++)
            {
                sendMsg(games[i].spectators[j], Menu());
                games[i].spectators[j] = 0;
            }
            games[i].nrOfspectators = 0;
            for (int j = i; j < nrOfGames; j++)
            {
                games[j] = games[j + 1];
            }
            nrOfGames--;

            break;
        }
    }
}
void endGame(int player)
{
    for (int i = 0; i < nrOfGames; i++)
    {
        if (games[i].player1 == player || games[i].player2 == player)
        {
            //Found the game
            printf("Found the game to end.\n");
            sendMsg(games[i].player1, Menu());
            sendMsg(games[i].player2, Menu());
            games[i].player1 = 0;
            games[i].player2 = 0;
            games[i].p1Set = false;
            games[i].p2Set = false;
            games[i].active = false;
            for (int j = 0; j < games[i].nrOfspectators; j++)
            {
                sendMsg(games[i].spectators[j], Menu());
                games[i].spectators[j] = 0;
            }
            games[i].nrOfspectators = 0;
            for (int j = i; j < nrOfGames; j++)
            {
                games[j] = games[j + 1];
            }
            nrOfGames--;

            break;
        }
    }
}
void checkForWinner(game &theGame)
{
    if (theGame.player1Score == 3)
    {
        //p1 wins
        sendMsg(theGame.player1, "You win\n");
        sendMsg(theGame.player2, "You loose\n");
        for (int j = 0; j < theGame.nrOfspectators; j++)
        {
            sendMsg(theGame.spectators[j], "p1 wins\n");
        }
        float time = theGame.p1totTime / theGame.rounds;
        printf("Is this a new best? %f\n", time);
        putInTop(theGame.p1totTime / theGame.rounds);
        endGame(theGame);
    }
    else if (theGame.player2Score == 3)
    {
        //p2 wins
        sendMsg(theGame.player1, "You loose\n");
        sendMsg(theGame.player2, "You win\n");
        for (int j = 0; j < theGame.nrOfspectators; j++)
        {
            sendMsg(theGame.spectators[j], "p2 wins\n");
        }
        float time = theGame.p1totTime / theGame.rounds;
        printf("Is this a new best? %f\n", time);
        putInTop(theGame.p2totTime / theGame.rounds);
        endGame(theGame);
    }
}
void handleGames()
{
    for (int i = 0; i < nrOfGames; i++)
    {
        if (games[i].rounds++ == games[i].maxRounds)
        {
            //p1 wins
            if (games[i].player1Score > games[i].player2Score)
            {
                sendMsg(games[i].player1, "The maximum number of rounds is reached, player 1 wins!\n");
                sendMsg(games[i].player2, "The maximum number of rounds is reached, player 1 wins!\n");
                for (int j = 0; j < games[i].nrOfspectators; j++)
                {
                    sendMsg(games[i].spectators[j], "The maximum number of rounds is reached, player 1 wins!\n");
                }
            }
            else if (games[i].player2Score > games[i].player1Score)
            {
                sendMsg(games[i].player1, "The maximum number of rounds is reached, player 2 wins!\n");
                sendMsg(games[i].player2, "The maximum number of rounds is reached, player 2 wins!\n");
                for (int j = 0; j < games[i].nrOfspectators; j++)
                {
                    sendMsg(games[i].spectators[j], "The maximum number of rounds is reached, player 2 wins!\n");
                }
            }
            else if (games[i].player1Score == games[i].player2Score)
            {
                sendMsg(games[i].player1, "The maximum number of rounds is reached, Draw!\n");
                sendMsg(games[i].player2, "The maximum number of rounds is reached, Draw!\n");
                for (int j = 0; j < games[i].nrOfspectators; j++)
                {
                    sendMsg(games[i].spectators[j], "The maximum number of rounds is reached, Draw!\n");
                }
            }
            endGame(games[i]);
            return;
        }
        else if (games[i].p1Set && games[i].p2Set)
        {
            games[i].p1totTime += games[i].p1time - games[i].startTime;
            games[i].p2totTime += games[i].p2time - games[i].startTime;
            printf("Player 1 awnserd in %ld seconds.\nPlayer 2 awnserd in %ld seconds.\n", games[i].p1time - games[i].startTime, games[i].p2time - games[i].startTime);
            printf("Boath players have set theri choises.\n");
            printf("P1 chose %d\nP2 chose %d\n", games[i].player1choise, games[i].player2choise);
            int win = Winner(games[i].player1choise, games[i].player2choise);
            printf("Thw winner of this round is %d\n", win);
            switch (win)
            {
            case 0:
                //Draw
                sendMsg(games[i].player1, "Draw\n");
                sendMsg(games[i].player2, "Draw\n");
                for (int j = 0; j < games[i].nrOfspectators; j++)
                {
                    sendMsg(games[i].spectators[j], "Draw\n");
                }
                break;
            case 1:
                //p1 socre
                games[i].player1Score++;
                sendMsg(games[i].player1, "You score\n");
                sendMsg(games[i].player2, "Your opponent scores\n");
                for (int j = 0; j < games[i].nrOfspectators; j++)
                {
                    sendMsg(games[i].spectators[j], "p1 score\n");
                }
                break;
            case 2:
                //p2 score
                games[i].player2Score++;
                sendMsg(games[i].player1, "Your opponent scores\n");
                sendMsg(games[i].player2, "You score\n");
                for (int j = 0; j < games[i].nrOfspectators; j++)
                {
                    sendMsg(games[i].spectators[j], "p2 score\n");
                }
                break;
            }
            string msg = "Start.\n1:Rock\n2:Paper\n3:Scissors\n";
            sendMsg(games[i].player1, msg);
            sendMsg(games[i].player2, msg);
            printf("P1 score: %d\nP2 score: %d\n", games[i].player1Score, games[i].player2Score);
            games[i].p1Set = false;
            games[i].p2Set = false;
            checkForWinner(games[i]);
            games[i].startTime = time(0);
            printf("Start timer.\n");
        }
    }
}
void test(int signum)
{
    for (int i = 0; i < nrOfGames; i++)
    {
        if (!games[i].active)
        {
            if (games[i].count > 0)
            {
                string msg = to_string(games[i].count--) + " seconds unitl start.\n";
                sendMsg(games[i].player1, msg);
                sendMsg(games[i].player2, msg);
                for (int j = 0; j < games[i].nrOfspectators; j++)
                {
                    sendMsg(games[i].spectators[j], msg);
                }
            }
            else
            {
                string msg = "Start.\n1:Rock\n2:Paper\n3:Scissors\n";
                sendMsg(games[i].player1, msg);
                sendMsg(games[i].player2, msg);
                for (int j = 0; j < games[i].nrOfspectators; j++)
                {
                    sendMsg(games[i].spectators[j], msg);
                }
                games[i].active = true;
                games[i].count = 0;
                games[i].startTime = time(0);
                printf("Start timer.\n");
            }
        }
        else
        {

            if (games[i].count++ == 3)
            {
                if (games[i].p1Set && games[i].p2Set)
                {
                    games[i].count = 0;
                    handleGames();
                }
                else if (!games[i].p1Set || !games[i].p2Set)
                {
                    if (!games[i].p1Set && !games[i].p2Set)
                    {
                        games[i].player1choise = 0;
                        games[i].player2choise = 0;
                        games[i].p1Set = true;
                        games[i].p2Set = true;
                        games[i].p1time = time(0);
                        games[i].p2time = time(0);
                    }
                    else if (!games[i].p1Set && games[i].p2Set)
                    {
                        games[i].player1choise = 0;
                        games[i].p1Set = true;
                        games[i].p1time = time(0);
                    }
                    else if (games[i].p1Set && !games[i].p2Set)
                    {
                        games[i].player2choise = 0;
                        games[i].p2Set = true;
                        games[i].p2time = time(0);
                    }
                    games[i].count = 0;
                    handleGames();
                }
            }
            else
            {
                string msg = "Game starts in " + to_string(4 - games[i].count) + "\n";
                sendMsg(games[i].player1, msg);
                sendMsg(games[i].player2, msg);
                for (int j = 0; j < games[i].nrOfspectators; j++)
                {
                    sendMsg(games[i].spectators[j], msg);
                }
            }
        }
    }
}
void newGame(int p1, int p2)
{
    removeFromQeueu(p1);
    removeFromQeueu(p2);
    games[nrOfGames].player1 = p1;
    games[nrOfGames].player2 = p2;
    games[nrOfGames].rounds = 0;
    //games[nrOfGames].active = true;
    //games[nrOfGames].startTime = time(0);
    games[nrOfGames].count = 3;
    games[nrOfGames].maxRounds = 12;
    games[nrOfGames].p1totTime = 0;
    games[nrOfGames].p2totTime = 0;

    games[nrOfGames].alarmTime.it_interval.tv_sec = 1;
    games[nrOfGames].alarmTime.it_interval.tv_usec = 1;
    games[nrOfGames].alarmTime.it_value.tv_sec = 1;
    games[nrOfGames].alarmTime.it_value.tv_usec = 1;
    /* Regiter a callback function, associated with the SIGALRM signal, which will be raised when the alarm goes of */
    signal(SIGALRM, test);
    setitimer(ITIMER_REAL, &games[nrOfGames].alarmTime, NULL); // Start/register the alarm.

    nrOfGames++;
}
void handleQueue()
{
    if (qeueu % 2 == 0 && qeueu > 0)
    {
        //There is an equal number of players in qeueu and the is more that 0 players in the qeueu
        //Start a new Game
        newGame(Gamequeue[0], Gamequeue[1]);
    }
}
void setPlayerChoise(int player, int choise)
{
    for (int i = 0; i < nrOfGames; i++)
    {
        if (player == games[i].player1)
        {
            if (!games[i].p1Set)
            {
                games[i].player1choise = choise;
                games[i].p1Set = true;
                games[i].p1time = time(0);
                printf("P1 set its choise: %d.\n", choise);
                break;
            }
        }
        else if (player == games[i].player2)
        {
            if (!games[i].p2Set)
            {
                games[i].player2choise = choise;
                games[i].p2Set = true;
                games[i].p2time = time(0);
                printf("P2 set its choise: %d.\n", choise);
                break;
            }
        }
    }
}
int checkPlayerStatus(int sock)
{
    //1 = in queue. 2 = in game
    int ret = 0;
    bool inQueue = false;
    for (int k = 0; k < qeueu; k++)
    {
        if (sock == Gamequeue[k])
        {
            inQueue = true;
            ret = 1;
            break;
        }
    }
    bool innGame = false;
    for (int k = 0; k < nrOfGames; k++)
    {
        if (sock == games[k].player1 || sock == games[k].player2)
        {
            innGame = true;
            ret = 2;
            break;
        }
    }
    bool spectating = false;
    for (int k = 0; k < nrOfGames; k++)
    {
        for (int j = 0; j < games[k].nrOfspectators; j++)
        {
            if (sock == games[k].spectators[j])
            {
                spectating = true;
                ret = 3;
                break;
            }
        }
    }
    bool spectatingQueue = false;
    for (int k = 0; k < nrInWatchQueue; k++)
    {
        if (sock == watchQueue[k])
        {
            spectatingQueue = true;
            ret = 4;
            break;
        }
    }
    return ret;
}
void addSpectator(int client, int gameNr)
{
    for (int k = 0; k < nrInWatchQueue; k++)
    {
        if (client == watchQueue[k])
        {
            //Found client
            for (int j = k; j < nrInWatchQueue; j++)
            {
                watchQueue[j] = watchQueue[j + 1];
            }
            //sendMsg(client, "You are being proccesed.\n");
            nrInWatchQueue--;
            break;
        }
    }
    if (games[gameNr].nrOfspectators < 10)
    {
        string msg = "You are now spectating game " + to_string(gameNr) + "\n";
        sendMsg(client, msg);
        games[gameNr].spectators[games[gameNr].nrOfspectators++] = client;
        sendMsg(games[gameNr].spectators[games[gameNr].nrOfspectators - 1], "Press 1, 2 or 3 to exit the game.\n");
    }
    else
    {
        sendMsg(client, "Cant fit any more spectators.\n");
    }
}
void addToSpectatorQueue(int client)
{
    if (nrInWatchQueue < 50)
    {
        watchQueue[nrInWatchQueue++] = client;
        sendMsg(client, "choose one of the games listed abowe with a number.\n");
    }
    else
    {
        sendMsg(client, "Cant fit any more spectators.\n");
    }
}
void removeFromWatch(int client)
{
    for (int i = 0; i < nrOfGames; i++)
    {
        for (int j = 0; j < games[i].nrOfspectators; j++)
        {
            if (games[i].spectators[j] == client)
            {
                for (int k = j; k < games[i].nrOfspectators; k++)
                {
                    games[i].spectators[k] = games[i].spectators[k + 1];
                }
                break;
            }
        }
    }
}

int main(int argc, char *argv[])
{

    //Takes ip and port in the following format:
    //ip:port
    if (argc != 2)
    {
        //We dont have the correct input. Exit the program
        printf("Invalid input.\n");
        exit(1);
    }
    char delim[] = ":";
    char *Desthost = strtok(argv[1], delim);
    char *Destport = strtok(NULL, delim);
    //Desthost should now have the ip in Desthost and port in Destport
    //Check if one of them might be NULL
    if (Desthost == NULL || Destport == NULL)
    {
        //One of the variables are null. Exit the program
        printf("Invalid input.\n");
        exit(1);
    }

    //Variables
    int listener;
    int newfd;
    int rv;
    int nrOfClient = 0;
    int yes = 1;
    int bytesRecived;
    string cmds[5] = {"OK\n", "1\n", "2\n", "3\n", "4\n"};

    struct addrinfo hint, *servinfo, *p;
    memset(&hint, 0, sizeof(hint));
    hint.ai_family = AF_UNSPEC;     //IPv4 or IPv6
    hint.ai_socktype = SOCK_STREAM; //TCP

    rv = getaddrinfo(Desthost, Destport, &hint, &servinfo);
    if (rv != 0)
    {
        //Getaddrinfo faild
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }

    //Loop trough all the information and try to make a sockt
    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0)
        {
            //Could not make socket try again
            continue;
        }
        //setsockoptions
        if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
        {
            printf("setsockopt.\n");
            exit(1);
        }
        //bind
        if (bind(listener, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(listener);
            printf("Could not bind.\n");
            continue;
        }
        //If we made it here we succesfully created a socket. Exit loop
        break;
    }
    freeaddrinfo(servinfo);
    //Check if the for loop did not make a socket
    if (p == NULL)
    {
        printf("Could not bind.\n");
        exit(1);
    }
    //Litser for connections
    if (listen(listener, MAXCLIENTS) < 0)
    {
        printf("Litse.\n");
        close(listener);
        exit(1);
    }

    fd_set master;
    FD_ZERO(&master);
    FD_SET(listener, &master);
    int fdmax = listener;

    char buf[255];
    memset(buf, 0, sizeof(buf));

    while (true)
    {
        fd_set copy = master;
        handleQueue();
        //handleGames(1);

        if (select(fdmax + 1, &copy, NULL, NULL, NULL) == -1)
        {
            if (errno == EINTR)
            {
                continue;
            }
            else
            {
                printf("select.\n");
                printf("%s\n", strerror(errno));
                close(listener);
                exit(1);
            }
        }

        for (int i = 0; i < fdmax + 1; i++)
        {
            if (FD_ISSET(i, &copy))
            {
                if (i == listener)
                {
                    //Accept a new connection
                    newfd = accept(listener, p->ai_addr, &p->ai_addrlen);
                    if (newfd < 0)
                    {
                        printf("Accept.\n");
                        continue;
                    }
                    else
                    {
                        FD_SET(newfd, &master);
                        nrOfClient++;
                        if (newfd > fdmax)
                        {
                            fdmax = newfd;
                        }
                        if (send(newfd, "1.0\n", strlen("1.0\n"), 0) < 0)
                        {
                            printf("Sending.\n");
                        }
                    }
                }
                else
                {
                    memset(buf, 0, sizeof(buf));
                    bytesRecived = recv(i, buf, sizeof(buf), 0);
                    if (checkPlayerStatus(i) == 4) //Spectating queue
                    {
                        //sendMsg(i, "Lokking for your game.\n");
                        sendMsg(i, buf);
                        if (bytesRecived <= 0)
                        {
                            //Remove from spectetingQueue
                            sendMsg(i, "Recived nothing.\n");
                        }
                        else
                        {
                            for (int j = 0; j < nrOfGames; j++)
                            {
                                //sendMsg(i, to_string(j));
                                //sendMsg(i, "\n");
                                if (atoi(buf) == j)
                                {
                                    //Found the game
                                    //sendMsg(i, "Found your game.\n");
                                    addSpectator(i, j);
                                    break;
                                }
                            }
                        }
                    }
                    else
                    {
                        //Accept a new message

                        if (bytesRecived <= 0)
                        {
                            if (checkPlayerStatus(i) == 1) //In queue
                            {
                                removeFromQeueu(i);
                            }
                            else if (checkPlayerStatus(i) == 2) //In game
                            {
                                printf("A player left the game.\n");

                                endGame(i);
                            }
                            else if (checkPlayerStatus(i) == 3) //spectating
                            {
                                removeFromWatch(i);
                            }
                            //printf("Remove client.\n");
                            nrOfClient--;
                            close(i);
                            FD_CLR(i, &master);
                        }
                        //Did the client support the protocol?
                        //If client sent "OK\n", send back alternatives of what the client can do.
                        /*Check for client responce
                    1: Play (If no players, put in qeueu. If odd number of players, start a new game. If even numbre of players, place in qeueu)
                       Give the option to leave qeueu 
                    2:Watch (List all the active games and give the cilet a choise)
                    0: Exit
                    */

                        if (strcmp(buf, cmds[0].c_str()) == 0)
                        { //cmds[0]=="OK\n"
                            //The client supports the prorocols
                            if (checkPlayerStatus(i) == 0) //Not in qeueu and not in game
                            {
                                //printf("The client supports the prorocols\n");
                                printf("New client.\n");
                                if (send(i, Menu().c_str(), Menu().length(), 0) < 0)
                                {
                                    printf("Sending.\n");
                                }
                            }
                        }
                        else if (strcmp(buf, cmds[1].c_str()) == 0)
                        { //cmds[1]=="1\n"
                            //The client wanst to play
                            memset(buf, 0, sizeof(buf));
                            if (checkPlayerStatus(i) == 0) //not in game or queue
                            {
                                //printf("The client wants to play\n");
                                Gamequeue[qeueu++] = i;
                                sendMsg(i, "Putting you in qeueu\n.Press 1 to exit qeueu.\n");
                                //printf("Nr of people in qeueu: %d\n", qeueu);
                            }
                            else if (checkPlayerStatus(i) == 1) //In queue
                            {
                                //Client wants to exit qeueu
                                removeFromQeueu(i);
                                sendMsg(i, Menu());
                            }
                            else if (checkPlayerStatus(i) == 2) //In game
                            {
                                setPlayerChoise(i, 1);
                                //handleGames(1);
                            }
                            else if (checkPlayerStatus(i) == 3) //spectating
                            {
                                removeFromWatch(i);
                                sendMsg(i, Menu());
                            }
                        }
                        else if (strcmp(buf, cmds[2].c_str()) == 0)
                        { //cmds[2]=="2\n"
                            //The client wats to watch
                            memset(buf, 0, sizeof(buf));
                            if (checkPlayerStatus(i) == 0) //Not in game or queue
                            {
                                //printf("The client wants to watch\n");
                                //printf("Nr of acrive games: %d\n", nrOfGames);
                                if (nrOfGames > 0)
                                {
                                    string allGames = "";
                                    for (int j = 0; j < nrOfGames; j++)
                                    {
                                        //printf("Game %d\n", j);
                                        allGames += "Game " + to_string(j) + "\n";
                                    }
                                    sendMsg(i, allGames);
                                    addToSpectatorQueue(i);
                                }
                                else
                                {
                                    sendMsg(i, "No active games.\n");
                                    sendMsg(i, Menu());
                                }
                            }
                            else if (checkPlayerStatus(i) == 1) //In queue
                            {
                            }
                            else if (checkPlayerStatus(i) == 2) //In game
                            {
                                setPlayerChoise(i, 2);
                                //handleGames(1);
                            }
                            else if (checkPlayerStatus(i) == 3) //spectating
                            {
                                removeFromWatch(i);
                                sendMsg(i, Menu());
                            }
                        }
                        else if (strcmp(buf, cmds[3].c_str()) == 0)
                        { //cmds[3]=="3\n"
                            memset(buf, 0, sizeof(buf));
                            if (checkPlayerStatus(i) == 0) //Not in game or queue
                            {
                                //The client wats to exit
                                //printf("The client wants to exit\n");
                                //printf("Remove client.\n");
                                nrOfClient--;
                                close(i);
                                FD_CLR(i, &master);
                            }
                            else if (checkPlayerStatus(i) == 1)
                            { //In queue
                            }
                            else if (checkPlayerStatus(i) == 2)
                            { //In game
                                setPlayerChoise(i, 3);
                                //handleGames(1);
                            }
                            else if (checkPlayerStatus(i) == 3) //spectating
                            {
                                removeFromWatch(i);
                                sendMsg(i, Menu());
                            }
                        }
                        else if (strcmp(buf, cmds[4].c_str()) == 0)
                        {
                            if (top5 != "")
                            {
                                sendMsg(i, top5);
                            }
                            else
                            {
                                sendMsg(i, "Nobody in the top 5.\n");
                            }

                            sendMsg(i, Menu());
                        }
                        else
                        {
                            //printf("Wrong command. You sent: %s", buf);
                        }

                        //printf("New message.\n");
                    }
                }
            }
        }
    }
    return 0;
}