
#pragma once
#pragma warning (disable:4996)

using namespace std;

#include <iostream>
#include "pthread.h"
#include "CInsim.h"

#define IS_PRODUCT_NAME "X-Y-Z Pos"
// The superadmin is the only person that can close the app with a chat command
// This is a USERNAME, so change it to your desire
#define SUPER_ADMIN "makakazo"
#define MAX_PLAYERS 32


// Global objects that must be accesed by all threads
CInsim insim;
pthread_mutex_t ismutex;


// Player struct. Contains all data related to a certain connected player.
struct player
{
    char UName[24];             // Username
    char PName[24];             // Player name
    byte UCID;                  // Connection ID
    byte PLID;                  // Player ID
    byte buttons;               // 0 = buttons not displaying/player off the grid; 1 = buttons displaying/player in the grid
    int X, Y;                   // X and Y coordinates in 2 meters units (used for calculating distances)
    unsigned long closest_dist; // Distance factor to the closest car
    char closest_PName[24];     // Player name of the closest car
};


// Main struct than contains all info stored/used by the application
struct global_info
{
    struct player players[MAX_PLAYERS];     // Array of players
    byte on_track;                          // 1 = on_track; 0 = track select screen
};


/**
* Send empty coordinates buttons to a player
* @param    player   Struct that contains all the info regarding a single player
*/
void send_buttons(struct player *player)
{
    // Send empty buttons
    cout << "(sending empty buttons)" << endl;
    pthread_mutex_lock (&ismutex);
    struct IS_BTN pack_btn;
    memset(&pack_btn, 0, sizeof(struct IS_BTN));
    pack_btn.Size = sizeof(struct IS_BTN);
    pack_btn.Type = ISP_BTN;
    pack_btn.ReqI = player->UCID;               // Must be non-zero, I'll just use UCID
    pack_btn.UCID = player->UCID;               // UCID of the player that will receive the button
    pack_btn.ClickID = 237;                     // X coordinate
    pack_btn.BStyle = ISB_LIGHT;                // Dark frame for window title
    pack_btn.L = 1;
    pack_btn.T = 70;
    pack_btn.W = 30;
    pack_btn.H = 10;
    insim.send_packet(&pack_btn);

    pack_btn.ClickID = 236;                     // Y coordinate
    pack_btn.L = 1;
    pack_btn.T = 80;
    pack_btn.W = 30;
    pack_btn.H = 10;
    insim.send_packet(&pack_btn);

    pack_btn.ClickID = 235;                     // Z coordinate
    pack_btn.L = 1;
    pack_btn.T = 90;
    pack_btn.W = 30;
    pack_btn.H = 10;
    insim.send_packet(&pack_btn);

    pack_btn.ClickID = 234;                     // Closest car
    pack_btn.L = 1;
    pack_btn.T = 100;
    pack_btn.W = 30;
    pack_btn.H = 10;
    insim.send_packet(&pack_btn);

    pthread_mutex_unlock (&ismutex);

    // The player now has empty buttons!
    player->buttons = 1;
}


/**
* Clear empty coordinates buttons for a player
* @param    player   Struct that contains all the info regarding a single player
*/
void clear_buttons(struct player *player)
{
    // The player is no longer close to anyone!
    player->closest_dist = UINT_MAX;

    // Clear empty buttons
    cout << "(clearing empty buttons)" << endl;
    pthread_mutex_lock (&ismutex);
    struct IS_BFN pack_bfn;
    memset(&pack_bfn, 0, sizeof(struct IS_BFN));
    pack_bfn.Size = sizeof(struct IS_BFN);
    pack_bfn.Type = ISP_BFN;
    pack_bfn.SubT = BFN_DEL_BTN;
    pack_bfn.UCID = player->UCID;               // UCID of the player that will clear the button
    pack_bfn.ClickID = 237;                     // X coordinate
    insim.send_packet(&pack_bfn);

    pack_bfn.ClickID = 236;                     // Y coordinate
    insim.send_packet(&pack_bfn);

    pack_bfn.ClickID = 235;                     // Z coordinate
    insim.send_packet(&pack_bfn);

    pack_bfn.ClickID = 234;                     // Closest car
    insim.send_packet(&pack_bfn);

    pthread_mutex_unlock (&ismutex);

    // The player now doesn't have empty buttons!
    player->buttons = 0;
}


/**
* All the distance calculating stuff
* @param    ginfo   Struct that contains all the info stored by the application
*/
void distances(struct global_info *ginfo)
{
    // Vars used in the formulas
    int X1, X2, Y1, Y2;
    unsigned long distance;

    // For each player, we check distance with the rest
    for (int i=0; i<MAX_PLAYERS-1; i++)
    {
        // Only check if "i" player is on track
        if (ginfo->players[i].buttons != 0)
        {
            X1 = ginfo->players[i].X;
            Y1 = ginfo->players[i].Y;

            // For each player starting from the next to "i"
            for (int j=i+1; j<MAX_PLAYERS; j++)
            {
                // Only check if this "j" is also on track
                if (ginfo->players[j].buttons != 0)
                {
                    X2 = ginfo->players[j].X;
                    Y2 = ginfo->players[j].Y;
                    distance = (X1-X2)*(X1-X2) + (Y1-Y2)*(Y1-Y2);

                    // If "j" is the closest to "i", update "i"'s data
                    if (distance < ginfo->players[i].closest_dist)
                    {
                        ginfo->players[i].closest_dist = distance;
                        strcpy(ginfo->players[i].closest_PName, ginfo->players[j].PName);
                    }
                    // If "i" is the closest to "j", update "j"'s data
                    if (distance < ginfo->players[j].closest_dist)
                    {
                        ginfo->players[j].closest_dist = distance;
                        strcpy(ginfo->players[j].closest_PName, ginfo->players[i].PName);
                    }
                }
            }
        }
    }

    // Send buttons with the name of the closest car
    for (int i=0; i<MAX_PLAYERS; i++)
    {
        if ((ginfo->players[i].closest_dist != 0)&&(ginfo->players[i].closest_dist < UINT_MAX))
        {
            pthread_mutex_lock (&ismutex);

            struct IS_BTN pack_btn;
            memset(&pack_btn, 0, sizeof(struct IS_BTN));
            pack_btn.Size = sizeof(struct IS_BTN);
            pack_btn.Type = ISP_BTN;
            pack_btn.ReqI = ginfo->players[i].UCID;     // Must be non-zero, I'll just use UCID
            pack_btn.UCID = ginfo->players[i].UCID;     // UCID of the player that will receive the button
            pack_btn.ClickID = 234;                     // Closest car
            strcpy(pack_btn.Text, ginfo->players[i].closest_PName);

            insim.send_packet(&pack_btn);

            pthread_mutex_unlock (&ismutex);
        }
    }
}


/**
* Actions to perform for each MCI packet
* @param    ginfo   Struct that contains all the info stored by the application
*/
void function_mci(struct global_info *ginfo)
{
    cout << " * IS_MCI (driver info)" << endl;
    int i;

    struct IS_MCI *pack_mci = (struct IS_MCI*)insim.udp_get_packet();

    // Basic structure of the buttons used later
    struct IS_BTN pack_btn;
    memset(&pack_btn, 0, sizeof(struct IS_BTN));
    pack_btn.Size = sizeof(struct IS_BTN);
    pack_btn.Type = ISP_BTN;

    // For each valid compcar struct ins the MCI packet
    for (int j=0; j < pack_mci->NumC; j++)
    {
        // Find the player in the player list
        for (i=0; i < MAX_PLAYERS; i++)
        {
            if (ginfo->players[i].PLID == pack_mci->Info[j].PLID)
            {
                cout << "ginfo->players[" << i << "].PLID found" << endl;
                cout << "UName: " << ginfo->players[i].UName << endl;
                cout << "UCID: " << (int)ginfo->players[i].UCID << endl;
                cout << "PLID: " << (int)ginfo->players[i].PLID << endl;
                break;
            }
        }

        // Update buttons with new coordinates
        pthread_mutex_lock (&ismutex);

        pack_btn.ReqI = ginfo->players[i].UCID;     // Must be non-zero, I'll just use UCID
        pack_btn.UCID = ginfo->players[i].UCID;     // UCID of the player that will receive the button

        pack_btn.ClickID = 237;                     // X coordinate
        strcpy(pack_btn.Text, "^7X: ");
        itoa(pack_mci->Info[j].X/65536, pack_btn.Text + 5, 10);
        strcat(pack_btn.Text, " m");
        insim.send_packet(&pack_btn);

        pack_btn.ClickID = 236;                     // Y coordinate
        strcpy(pack_btn.Text, "^7Y: ");
        itoa(pack_mci->Info[j].Y/65536, pack_btn.Text + 5, 10);
        strcat(pack_btn.Text, " m");
        insim.send_packet(&pack_btn);

        pack_btn.ClickID = 235;                     // Z coordinate
        strcpy(pack_btn.Text, "^7Z: ");
        itoa(pack_mci->Info[j].Z/65536, pack_btn.Text + 5, 10);
        strcat(pack_btn.Text, " m");
        insim.send_packet(&pack_btn);

        pthread_mutex_unlock (&ismutex);

        // Save X and Y coordinates for calculating distance. The "/131072" is to prevent overflow later
        ginfo->players[i].X = pack_mci->Info[j].X/131072;
        ginfo->players[i].Y = pack_mci->Info[j].Y/131072;

        // Set the current closest distance of this player to maximum
        ginfo->players[i].closest_dist = UINT_MAX;

        // If this compcar is the last of the sequence, we'll do all the distance calculating stuff
        if (pack_mci->Info[j].Info >= CCI_LAST)
            distances(ginfo);
    }
}


/**
* UDP loop thread. This thread handles the MCI packets
* @param
*/
void *udp_loop_thread(void *ginfo)
{
    // UDP loop. Repeat forever until main thread closes this thread
    while(1)
    {
        // Get next packet ready
        if (insim.udp_next_packet() < 0)
        {
            cerr << "\n * Error getting next UDP packet * " << endl;
            continue;
        }

        // Peek the next packet to know its type, and call the matching function
        cout << " \n***********************************************************" << endl;
        cout << " * Received UDP packet of type: " << (int)insim.udp_peek_packet() << endl;

        function_mci((struct global_info*)ginfo);
    }
}


/**
* Actions performed when someone types something in the chat (packet IS_MSO)
* @param    ginfo   Struct that contains all the info stored by the application
* @return   1 in normal cases, -1 if the super admin requested !exit
*/
int case_mso (struct global_info *ginfo)
{
    cout << " * IS_MSO (chat message)" << endl;
    int i;

    struct IS_MSO *pack_mso = (struct IS_MSO*)insim.get_packet();

    // Find the player that wrote in the chat
    for (i=0; i < MAX_PLAYERS; i++)
    {
        if (ginfo->players[i].UCID == pack_mso->UCID)
        {
            cout << "ginfo->players[" << i << "].UCID found" << endl;
            cout << "UName: " << ginfo->players[i].UName << endl;
            cout << "UCID: " << (int)ginfo->players[i].UCID << endl;
            cout << "PLID: " << (int)ginfo->players[i].PLID << endl;
            cout << "Msg: " << pack_mso->Msg << endl;
            break;
        }
    }

    // !exit can only be requested by the "super admin"
    if (strncmp(pack_mso->Msg + ((unsigned char)pack_mso->TextStart), "!exit", 6) == 0)
    {
        cout << "(!exit requested)" << endl;
        if (strcmp(ginfo->players[i].UName, SUPER_ADMIN) != 0)
            return 1;

        pthread_mutex_lock (&ismutex);
        struct IS_MST pack_mst;
        memset(&pack_mst, 0, sizeof(struct IS_MST));
        pack_mst.Size = sizeof(struct IS_MST);
        pack_mst.Type = ISP_MST;
        strcpy(pack_mst.Msg, "^3* Closing application (requested by admin) *");
        insim.send_packet(&pack_mst);

        pthread_mutex_unlock (&ismutex);
        return -1;
    }

    return 1;
}


/**
* New connection. Add the player to the list and send welcome text and help screen.
* @param    ginfo   Struct that contains all the info stored by the application
*/
void case_ncn (struct global_info *ginfo)
{
    cout << " * IS_NCN (new connection)" << endl;
    int i;

    struct IS_NCN *pack_ncn = (struct IS_NCN*)insim.get_packet();

    // UCID == 0 means it's the host, don't add it to the array.
    // I use UCID=0 for empty slots within the player array.
    if (pack_ncn->UCID == 0)
    {
        cout << "(Host connected, not adding him to array...)" << endl;
        return;
    }

    // Stop when the first empty slot is found, checking UCID==0
    for (i=0; i<MAX_PLAYERS; i++)
        if (ginfo->players[i].UCID == 0)
            break;

    // If i reached MAX_PLAYERS then we have more players than what the application expects.
    // MAX_PLAYERS should be set to the actual maximum number of drivers that the server allows.
    if (i == MAX_PLAYERS)
    {
        cerr << "NO MORE PEOPLE ALLOWED!!!" << endl;
        return;
    }

    // Copy all the player data we need into the ginfo->players[] array
    cout << "(Initializing player data into global struct)" << endl;
    strcpy(ginfo->players[i].UName, pack_ncn->UName);
    strcpy(ginfo->players[i].PName, pack_ncn->PName);
    ginfo->players[i].UCID = pack_ncn->UCID;

    cout << "NEW player connected!" << endl;
    cout << "UName:" << ginfo->players[i].UName << endl;
    cout << "UCID: " << (int)ginfo->players[i].UCID << endl;
    cout << "PLID: " << (int)ginfo->players[i].PLID << endl;

    // Send a welcome message
    pthread_mutex_lock (&ismutex);
    struct IS_MTC pack_mtc;
    memset(&pack_mtc, 0, sizeof(struct IS_MTC));
    pack_mtc.Size = sizeof(struct IS_MTC);
    pack_mtc.Type = ISP_MTC;
    pack_mtc.UCID = ginfo->players[i].UCID;
    strcpy(pack_mtc.Text , "^3Welcome! ^7X-Y-Z Positioning ^3is running...");
    insim.send_packet(&pack_mtc);
    pthread_mutex_unlock (&ismutex);
}

/**
* Player changed his pilot name (player name). Update the affected data.
* @param    ginfo   Struct that contains all the info stored by the application
*/
void case_cpr (struct global_info *ginfo)
{
    cout << " * IS_CPR (player rename)" << endl;
    int i;

    struct IS_CPR *pack_cpr = (struct IS_CPR*)insim.get_packet();

    // Find the player
    for (i=0; i < MAX_PLAYERS; i++)
    {
        if (ginfo->players[i].UCID == pack_cpr->UCID)
        {
            cout << "ginfo->players[" << i << "].PLID found" << endl;
            cout << "UName: " << ginfo->players[i].UName << endl;
            cout << "UCID: " << (int)ginfo->players[i].UCID << endl;
            cout << "PLID: " << (int)ginfo->players[i].PLID << endl;
            break;
        }
    }

    // Update the player name
    strcpy(ginfo->players[i].PName, pack_cpr->PName);
    cout << "(Player changed name to " << ginfo->players[i].PName << ")" << endl;
}

/**
* New player joining race or leaving pits
* @param    ginfo   Struct that contains all the info stored by the application
*/
void case_npl (struct global_info *ginfo)
{
    cout << " * IS_NPL (joining race or leaving pits)" << endl;
    int i;

    struct IS_NPL *pack_npl = (struct IS_NPL*)insim.get_packet();

    // Find player using UCID and update his PLID
    for (i=0; i < MAX_PLAYERS; i++)
    {
        if (ginfo->players[i].UCID == pack_npl->UCID)
        {
            cout << "ginfo->players[" << i << "].UCID found" << endl;
            cout << "UName: " << ginfo->players[i].UName << endl;
            cout << "UCID: " << (int)ginfo->players[i].UCID << endl;
            ginfo->players[i].PLID = pack_npl->PLID;
            cout << "PLID: " << (int)ginfo->players[i].PLID << " (new one if joining, or the same he had if exiting pits)" << endl;

            // Send buttons if player is on track
            if ((ginfo->on_track == 1)&&(ginfo->players[i].buttons == 0))
                send_buttons(&ginfo->players[i]);

            break;
        }
    }
}

/**
* Player leaves race (spectates - loses slot)
* @param    ginfo   Struct that contains all the info stored by the application
*/
void case_pll (struct global_info *ginfo)
{
    cout << " * IS_PLL (player leaves race)" << endl;
    int i;

    struct IS_PLL *pack_pll = (struct IS_PLL*)insim.get_packet();

    // Find player and set his PLID to 0, and clear buttons if he's on track
    for (i=0; i < MAX_PLAYERS; i++)
    {
        if (ginfo->players[i].PLID == pack_pll->PLID)
        {
            cout << "ginfo->players[" << i << "].PLID found" << endl;
            cout << "UName: " << ginfo->players[i].UName << endl;
            cout << "UCID: " << (int)ginfo->players[i].UCID << endl;
            ginfo->players[i].PLID = 0;
            cout << "PLID: " << (int)ginfo->players[i].PLID << " (set to zero)" << endl;

            // Clear buttons if the player has them on screen
            if (ginfo->players[i].buttons == 1)
                clear_buttons(&ginfo->players[i]);

            break;
        }
    }
}

/**
* Connection left
* @param    ginfo   Struct that contains all the info stored by the application
*/
void case_cnl (struct global_info *ginfo)
{
    cout << " * IS_CNL (connection left)" << endl;
    int i;

    struct IS_CNL *pack_cnl = (struct IS_CNL*)insim.get_packet();

    // Find player and set the whole player struct he was using to 0
    for (i=0; i < MAX_PLAYERS; i++)
    {
        if (ginfo->players[i].UCID == pack_cnl->UCID)
        {
            cout << "ginfo->players[" << i << "].UCID found: exiting" << endl;
            cout << "UName: " << ginfo->players[i].UName << endl;
            cout << "UCID: " << (int)ginfo->players[i].UCID << endl;
            cout << "PLID: " << (int)ginfo->players[i].PLID << endl;
            memset(&ginfo->players[i], 0, sizeof(ginfo->players[i]));
            cout << "ginfo->players[" << i << "].UCID found: CHECKING" << endl;
            cout << "UName: " << ginfo->players[i].UName << endl;
            cout << "UCID: " << (int)ginfo->players[i].UCID << endl;
            cout << "PLID: " << (int)ginfo->players[i].PLID << endl;
            break;
        }
    }
}


/**
* Race reorder. This means a session is restarting.
* @param    ginfo   Struct that contains all the info stored by the application
*/
void case_reo (struct global_info *ginfo)
{
    cout << " * IS_REO (race reorder/restarting)" << endl;
    int i;

    // We are now on-track
    ginfo->on_track = 1;

    // Check players on track to send empty buttons
    for (i=0; i < MAX_PLAYERS; i++)
    {
        if (ginfo->players[i].PLID != 0)
        {
            send_buttons(&ginfo->players[i]);
        }
    }
}


/**
* IS_TINY packet received.
* @param    ginfo   Struct that contains all the info stored by the application
*/
void case_tiny (struct global_info *ginfo)
{
    cout << " * IS_TINY (multipurpose)" << endl;
    int i;

    struct IS_TINY *pack_tiny = (struct IS_TINY*)insim.get_packet();

    // We only want IS_TINY's announcing race end; otherwise we ignore them and return
    if ((pack_tiny->ReqI != 0) || (pack_tiny->SubT != TINY_REN))
        return;

    // We are now not on-track
    ginfo->on_track = 0;

    // Check players with buttons to clear them
    for (i=0; i < MAX_PLAYERS; i++)
    {
        if (ginfo->players[i].buttons == 1)
        {
            clear_buttons(&ginfo->players[i]);
        }
    }
}

/**
* Initialize server
*/
void config_server()
{
    // End race if any is running. Some error messages are thrown during the /end countdown
    pthread_mutex_lock (&ismutex);

    struct IS_MST pack_mst;
    memset(&pack_mst, 0, sizeof(struct IS_MST));
    pack_mst.Size = sizeof(struct IS_MST);
    pack_mst.Type = ISP_MST;
    memcpy(pack_mst.Msg, "/end", 4);
    insim.send_packet(&pack_mst);

    pthread_mutex_unlock (&ismutex);
}


/**
* Main program. Receives arguments used in the connection to the server.
* @param    argc    Number of arguments in the application call
* @param    argv    Arguments received in the application call.
*/
int main (int argc, char **argv)
{
    // Initialize the mutex var
    pthread_mutex_init(&ismutex, NULL);

    // Arguments check
    if (argc < 4)
    {
        cerr << " * Not enough arguments! *" << endl;
        cerr << " Usage:\n \"executable\" (ip) (port) (admin_pass)" << endl;
        cerr << "\n PRESS RETURN KEY TO FINISH" << endl;
        getchar();
        return -1;
    }

    // Error check var used when calling next_packet()
    int error_ch;

    // Create the main global_info struct and clear it completely to zeroes.
    struct global_info ginfo;
    memset(&ginfo, 0, sizeof(struct global_info));

    // We create an IS_VER packet to receive the response from insim.init()
    struct IS_VER pack_ver;

    // Initialize the connection to InSim:
    // argv[1] -> Host IP
    // argv[2] -> Host port (needs to be converted to an unsigned short (word))
    // argv[3] -> Admin password
    if (insim.init(argv[1], (word)atoi(argv[2]), IS_PRODUCT_NAME, argv[3], &pack_ver, '!', ISF_MCI, 1000, 60000) < 0)
    {
        cerr << "\n * Error during initialization * " << endl;
        cerr << "\n PRESS RETURN KEY TO FINISH" << endl;
        getchar();
        return -1;
    }

    cout << "\n********************************\n" << endl;
    cout << "LFS Version: " << pack_ver.Version << endl;
    cout << "InSim Version: " << pack_ver.InSimVer << endl;
    cout << "\n********************************\n" << endl;

    // Initialize server
    config_server();

    // This IS_TINY packet is used to request several things from the server before the main loop
    struct IS_TINY pack_requests;
    memset(&pack_requests, 0, sizeof(struct IS_TINY));
    pack_requests.Size = sizeof(struct IS_TINY);
    pack_requests.Type = ISP_TINY;
    pack_requests.ReqI = 1;
    pack_requests.SubT = TINY_NCN;      // Request all connections to store their user data
    insim.send_packet(&pack_requests);
    cout << "Connections request packet sent!" << endl;

    pack_requests.SubT = TINY_NPL;      // Request all players in-grid to know their PLID
    insim.send_packet(&pack_requests);
    cout << "Player info request packet sent!" << endl;

    // Create the thread that will contain the loop for UDP packets
    int rc;
    pthread_t thread;
    rc = pthread_create(&thread, NULL, udp_loop_thread, (void*)&ginfo);

    if (rc)
    {
        insim.isclose();
        cerr << "\n * Error creating UDP thread * " << endl;
        cerr << "\n PRESS RETURN KEY TO FINISH" << endl;
        getchar();
        return rc;
    }

    // Main loop. When ok is 0 we'll exit this loop
    int ok = 1;
    while(ok > 0)
    {
        // Get next packet ready
        if (error_ch = insim.next_packet() < 0)
        {
            cerr << "\n * Error getting next packet * " << endl;
            cerr << "\n PRESS RETURN KEY TO FINISH" << endl;
            getchar();
            return error_ch;
        }

        // Peek the next packet to know its type, and call the matching function
        cout << " \n***********************************************************" << endl;
        cout << " * Received packet of type: " << (int)insim.peek_packet() << endl;
        switch(insim.peek_packet())
        {
            case ISP_MSO:
                ok = case_mso (&ginfo); // in case_mso the user can request to close the application
                break;
            case ISP_NPL:
                case_npl (&ginfo);
                break;
            case ISP_NCN:
                case_ncn (&ginfo);
                break;
            case ISP_PLL:
                case_pll (&ginfo);
                break;
            case ISP_CNL:
                case_cnl (&ginfo);
                break;
            case ISP_CPR:
                case_cpr (&ginfo);
                break;
            case ISP_REO:
                case_reo (&ginfo);
                break;
            case ISP_TINY:
                case_tiny (&ginfo);
                break;
        }
    }

    // Cancel the UDP loop thread and destroy the mutex var
    pthread_cancel(thread);
    pthread_mutex_destroy(&ismutex);

    // The main loop ended: close connection to insim
    if (insim.isclose() < 0)
    {
        cerr << "\n * Error closing connection * " << endl;
        cerr << "\n PRESS RETURN KEY TO FINISH" << endl;
        getchar();
        return -1;
    }

    return 0;
}
