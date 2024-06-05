/*
Documentation:

Topic         : Final Project Pemrograman Lanjut

Tanggal       : 24 Mei 2024
Grup          : 26
Anggota       : Grace Yunike Margaretha Sitorus
                Laura Fawzia Sambowo
NPM           : 2306267031
                2306260145

Nama Program  : Sweet n' Sour
Version       : 2.0 
Description   : Program Sweet n' Sour merupakan sebuah program yang memungkinkan pengguna untuk mencari pasangan atau teman berdasarkan kesamaan minat dan kepribadian mereka. 
                Program ini dapat membantu pengguna menemukan koneksi yang lebih dekat dengan orang lain yang memiliki minat dan kepribadian yang serupa.

*/

#include <omp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_PROFILES 100

// Untuk memberi warna pada program
#define RESET       "\033[0m"
#define PINK        "\033[95m"
#define ORANGE      "\033[38;5;208m"
#define YELLOW      "\033[93m" 

// Struktur data untuk merepresentasikan profil pengguna
typedef struct {
    char full_name[100];
    char username[50];
    char password[50];
    int age;
    char kepribadian[50];
    char minat[100];
    char hobby[100];
    char activity[10];
    char gender;
    char social_media[100];
} Profile;

// Node untuk linked list
typedef struct Node {
    Profile data;
    struct Node *next;
} Node;

// Tabel kepribadian yang sesuai
const char *matchingPersonalities[16][2] = {
    {"INTJ", "ENFP"}, {"INTP", "ENTJ"}, {"ENTJ", "INTP"}, {"ENTP", "INFJ"}, {"INFJ", "ENTP"}, {"INFP", "ENFJ"}, {"ENFJ", "INFP"}, {"ENFP", "INTJ"}, {"ISTJ", "ESFP"}, {"ISFJ", "ESFJ"}, {"ESTJ", "ISFJ"}, {"ESFJ", "ISFP"}, {"ISTP", "ESTJ"}, {"ISFP", "ESFJ"}, {"ESTP", "ISTJ"}, {"ESFP", "ISTJ"}};

// Deklarasi fungsi
void clearScreen();
void loginMenu(Profile profiles[], int *numProfiles);

// Fungsi untuk mencocokkan kepribadian
int matchPersonality(const char *p1, const char *p2) {
    for (int i = 0; i < 16; i++) {
        if ((strcmp(matchingPersonalities[i][0], p1) == 0 && strcmp(matchingPersonalities[i][1], p2) == 0) ||
            (strcmp(matchingPersonalities[i][1], p1) == 0 && strcmp(matchingPersonalities[i][0], p2) == 0)) {
            return 1;
        }
    }
    return 0;
}

// Fungsi untuk menambahkan profil pengguna yang cocok ke dalam linked list
void addToMatchedProfilesList(Node **head, Profile matchedProfile) {
    Node *newNode = (Node *)malloc(sizeof(Node));
    if (newNode == NULL) {
        printf("Error: Memori tidak cukup.\n");
        exit(EXIT_FAILURE);
    }
    newNode->data = matchedProfile;
    newNode->next = *head;
    *head = newNode;
}

//Fungsi untuk menyimpan data match pada file matched.txt
void saveMatchedToFile(Profile currentUser, Node *head) {
    FILE *file = fopen("matched.txt", "a");
    if (file == NULL) {
        printf("Error: File tidak dapat dibuka.\n");
        return;
    }

    Node *current = head;
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    fprintf(file, "%s, %s, %02d-%02d-%04d %02d:%02d:%02d\n", currentUser.username, current->data.username, t->tm_mday, t->tm_mon + 1, t->tm_year + 1900, t->tm_hour, t->tm_min, t->tm_sec);

    // file format: username_one, username_two, time

    fclose(file);
}

// Fungsi untuk membersihkan linked list dari memori
void freeMatchedProfilesList(Node *head) {
    Node *temp;
    while (head != NULL) {
        temp = head;
        head = head->next;
        free(temp);
    }
}

// Fungsi untuk menghitung skor kesamaan profil
int calculateSimilarityScore(Profile user1, Profile user2) {
    int score = 0;

    //membandingkan minat
    if (strcmp(user1.minat, user2.minat) == 0) {
        score += 10;
    }

    // Membandingkan hobi
    if (strcmp(user1.hobby, user2.hobby) == 0) {
        score += 10;
    }

    // Membandingkan minat dan hobi
    if (strcmp(user1.minat, user2.hobby) == 0) {
        score += 10;
    } else if (strcmp(user1.hobby, user2.minat) == 0) {
        score += 10;
    }

    // Membandingkan kepribadian
    if (strcmp(user1.kepribadian, user2.kepribadian) == 0 || matchPersonality(user1.kepribadian, user2.kepribadian)) {
        score += 10;
    }

    // Membandingkan umur
    if (user1.age >= user2.age - 5 && user1.age <= user2.age + 5) {
        score += 10;
    }

    // Membandingkan preferensi aktivitas (indoors atau outdoors)
    if (strcmp(user1.activity, user2.activity) == 0) {
        score += 10;
    }

    return score;
}

// Fungsi untuk mencocokkan profil secara otomatis (Fitur 5)
void autoMatchProfiles(Profile currentUser, Profile profiles[], int numProfiles, Node **matchedProfilesList) {
    #pragma omp parallel
    {
    #pragma omp master
        {
            printf("\nMencocokkan profil pengguna secara otomatis...\n");
        }

    #pragma omp for
        for (int i = 0; i < numProfiles; i++) {
            int score = calculateSimilarityScore(currentUser, profiles[i]);
            if (score > 0 && strcmp(currentUser.username, profiles[i].username) != 0) {
    #pragma omp critical
                {
                    if (currentUser.gender != profiles[i].gender) {
                        addToMatchedProfilesList(matchedProfilesList, profiles[i]);
                    }
                }
            }
        }

    #pragma omp single
        {
            printf("Profil pengguna yang cocok telah ditemukan.\n");
            Node *current = *matchedProfilesList;
            int index = 1;
            while (current != NULL) {
                printf("%d. %-20s (%s), %d tahun - Similarity Score: %d\n", index, current->data.full_name, current->data.username, current->data.age, calculateSimilarityScore(currentUser, current->data));
                current = current->next;
                index++;
            }
            printf("\nApakah Anda ingin menerima match mereka? (yes/no): ");
            char response[4];
            scanf("%s", response);
            if (strcmp(response, "yes") == 0) {
                // Lanjutkan dengan menambahkan fitur menerima atau menolak match satu per satu
                current = *matchedProfilesList;
                while (current != NULL) {
                    printf("Menerima match dengan %s? (yes/no): ", current->data.full_name);
                    scanf("%s", response);
                    if (strcmp(response, "yes") == 0) {
                        // Lanjutkan dengan logika untuk menerima match
                        // Misalnya, tambahkan ke daftar teman atau lakukan tindakan lain
                        printf("Match dengan %s diterima!\n", current->data.full_name);

                        // print data to file
                        saveMatchedToFile(currentUser, current);
                    } else {
                        // Lanjutkan dengan logika untuk menolak match
                        // Misalnya, lanjutkan ke profil berikutnya atau lakukan tindakan lain
                        printf("Match dengan %s ditolak.\n", current->data.full_name);
                    }
                    current = current->next;
                }
            } else {
                printf("Anda menolak untuk menerima match.\n");
            }
        }
    }
}

// Fungsi untuk menghubungi match dan menampilkan sosial media
void contactMatch(Profile *currentUser) {
    clearScreen();
    printf("Kontak Match:\n");
    printf("=======================================================================\n");

    Node *matchedProfilesList = NULL;
    Profile profiles[MAX_PROFILES];
    int numProfiles;
    loadProfilesFromFile(profiles, &numProfiles, "profiles.txt");
    autoMatchProfiles(*currentUser, profiles, numProfiles, &matchedProfilesList);

    if (matchedProfilesList == NULL) {
        printf("Anda belum memiliki match.\n");
    } else {
        Node *current = matchedProfilesList;
        int index = 1;
        while (current != NULL) {
            printf("%d. %s (%s)\n", index, current->data.full_name, current->data.username);
            current = current->next;
            index++;
        }

        printf("Masukkan nomor match yang ingin Anda hubungi: ");
        int choice;
        scanf("%d", &choice);

        current = matchedProfilesList;
        for (int i = 1; i < choice && current != NULL; i++) {
            current = current->next;
        }

        if (current != NULL) {
            printf("Informasi sosial media dari %s:\n", current->data.full_name);
            printf("Sosial Media: %s\n", current->data.social_media);
        } else {
            printf("Pilihan tidak valid.\n");
        }
    }

    freeMatchedProfilesList(matchedProfilesList);
    printf("Tekan Enter untuk kembali...");
    getchar();
    getchar();
}

// Fungsi untuk memuat profil pengguna dari file
int loadProfilesFromFile(Profile profiles[], int *numProfiles, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error: File tidak dapat dibuka.\n");
        return 0;
    }

    int count = 0;

    while (fscanf(file, "%99[^,], %49[^,], %49[^,], %d, %49[^,], %49[^,], %49[^,], %9[^,], %c\n",
            profiles[count].full_name, profiles[count].username, profiles[count].password, &profiles[count].age, profiles[count].kepribadian,
            profiles[count].minat, profiles[count].hobby, profiles[count].activity, &profiles[count].gender) != EOF) {
        count++;
    }
    *numProfiles = count;
    fclose(file);
    return 1;
}

// Fungsi untuk menyimpan profil pengguna ke file
void saveProfilesToFile(Profile profiles[], int numProfiles, const char *filename) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        printf("Error: File tidak dapat dibuka.\n");
        return;
    }

    for (int i = 0; i < numProfiles; i++) {
        fprintf(file, "%s, %s, %s, %d, %s, %s, %s, %s, %c\n",
                profiles[i].full_name, profiles[i].username, profiles[i].password, profiles[i].age, profiles[i].kepribadian,
                profiles[i].minat, profiles[i].hobby, profiles[i].activity, profiles[i].gender);
    }
    fclose(file);
}

// Fungsi untuk login, menampilkan profil, melihat matches, serta menghapus akun
void userMenu(Profile *currentUser, Profile profiles[], int *numProfiles) {
    Node *matchedProfilesList = NULL;
    int choice;

    while (1) {
        clearScreen();
        printf("============================================\n");
        printf("|             Selamat datang,              |\n");
        printf("|               %-22s     |\n", currentUser->full_name);
        printf("============================================\n");
        printf("|               Menu Utama                 |\n");
        printf("============================================\n");
        printf("| 1. Lihat Profil                          |\n");
        printf("| 2. Cek Matches                           |\n");
        printf("| 3. Kontak Match " YELLOW "(Coming Soon!)" RESET "           |\n");
        printf("| 4. Hapus Akun                            |\n");
        printf("| 5. Logout                                |\n");
        printf("============================================\n");
        printf("Pilihan Anda: ");
        scanf("%d", &choice);

        switch (choice) {
        case 1:
            clearScreen();
            printf("Profil Anda:\n");
            printf("=======================================================================\n");
            printf("| Nama Lengkap: %-20s | Username: %-20s |\n", currentUser->full_name, currentUser->username);
            printf("| Umur: %-28d | Kepribadian: %-17s |\n", currentUser->age, currentUser->kepribadian);
            printf("| Minat: %-27s | Hobi: %-24s |\n", currentUser->minat, currentUser->hobby);
            printf("| Preferensi Aktivitas: %-12s | Jenis Kelamin: %-15c |\n", currentUser->activity, currentUser->gender);
            printf("=======================================================================\n");
            printf("Tekan Enter untuk kembali...");
            getchar();
            getchar();
            break;
        case 2: 
            autoMatchProfiles(*currentUser, profiles, *numProfiles, &matchedProfilesList);
            freeMatchedProfilesList(matchedProfilesList);
            printf("Tekan Enter untuk kembali...");
            getchar();
            getchar();
            break;
        case 3:
            contactMatch(currentUser);
            break;
        case 4:
            // Menghapus akun
            char verify [5];
            printf("Apakah Anda yakin ingin menghapus akun? (Y/N)");
            scanf(" %[^\n]", verify);
            if (strcasecmp(verify, "Y") == 0) {
                for (int i = 0; i < *numProfiles; i++) {
                    if (strcmp(profiles[i].username, currentUser->username) == 0) {
                        for (int j = i; j < *numProfiles - 1; j++) {
                            profiles[j] = profiles[j + 1];
                        }
                        (*numProfiles)--;
                        saveProfilesToFile(profiles, *numProfiles, "profiles.txt");
                        printf("Akun Anda telah dihapus.\n");
                        printf("Tekan Enter untuk kembali ke menu utama...");
                        getchar();
                        getchar();
                        return;
                    }
                }
                break;
            } else {
                printf("Terima kasih telah memilih untuk tetap menggunakan" ORANGE " Sweet n' Sour!\n" RESET);
                printf("Tekan Enter untuk kembali ke menu user...");
                break;
            }
        case 5:
            return;
        default:
            printf("Pilihan tidak valid!\n");
            printf("Tekan Enter untuk kembali...");
            getchar();
            getchar();
        }
    }
}

// Fungsi untuk signup pengguna baru
void signUp(Profile profiles[], int *numProfiles) {
    Profile newUser;
    int isUsernameTaken = 0;

    clearScreen();
    printf("\n===== SIGN UP =====\n");

    while (1) {
        isUsernameTaken = 0;
        printf("Nama Lengkap: ");
        scanf(" %[^\n]", newUser.full_name);
        printf("Username: ");
        scanf(" %[^\n]", newUser.username);

        for (int i = 0; i < *numProfiles; i++) {
            if (strcmp(profiles[i].username, newUser.username) == 0) {
                isUsernameTaken = 1;
                printf("Username telah digunakan, silakan coba username lain.\n");
                break;
            }
        }

        if (!isUsernameTaken) {
            break;
        }
    }

    printf("Password: ");
    scanf(" %[^\n]", newUser.password);
    printf("Umur: ");
    scanf("%d", &newUser.age);
    printf("Kepribadian (16 Personalities): ");
    scanf(" %[^\n]", newUser.kepribadian);
    printf("Minat: ");
    scanf(" %[^\n]", newUser.minat);
    printf("Hobi: ");
    scanf(" %[^\n]", newUser.hobby);
    printf("Apakah Anda lebih suka kegiatan dalam ruangan (Indoor) atau luar ruangan (Outdoor)? (Indoor/Outdoor): ");
    scanf(" %[^\n]", newUser.activity);
    getchar();
    printf("Jenis Kelamin (M/F): ");
    scanf("%c", &newUser.gender);

    profiles[*numProfiles] = newUser;
    (*numProfiles)++;
    saveProfilesToFile(profiles, *numProfiles, "profiles.txt");
    printf("\nAkun pengguna berhasil dibuat!\n");
    printf("Tekan Enter untuk melanjutkan ke menu login...");
    getchar();
    getchar(); // Untuk menangkap karakter newline yang tersisa dari scanf
    loginMenu(profiles, numProfiles);
}

// Fungsi untuk login sebagai admin atau user
void loginMenu(Profile profiles[], int *numProfiles) {
    char username[50], password[50];
    int loginSuccess = 0;
    Profile *currentUser = NULL;

    clearScreen();
    printf("===== LOGIN =====\n");
    printf("Username: ");
    scanf(" %[^\n]", username);
    printf("Password: ");
    scanf(" %[^\n]", password);

    for (int i = 0; i < *numProfiles; i++) {
        if (strcmp(profiles[i].username, username) == 0 && strcmp(profiles[i].password, password) == 0) {
            loginSuccess = 1;
            currentUser = &profiles[i];
            break;
        }
    }

    if (loginSuccess) {
        if (strcmp(username, "admin") == 0) {
            char key[10];
            printf("\nSecurity Question Admin.\n");
            printf(PINK "Apakah anda percaya atas cinta pada pandangan yang pertama? (Y/N)\n" RESET);
            printf("Jawaban: ");
            scanf(" %[^\n]", key);
            if (strcasecmp(key, "Y") == 0) {
                printf("Login sebagai Admin berhasil.\n");
                printf("Data Pengguna:\n");
                printf("----------------------------------------------------------------------------------------------------------------------------\n");
                printf("%-20s %-20s %-10s %-15s %-15s %-15s %-15s %-5s\n", "Nama Lengkap", "Username", "Umur", "Kepribadian", "Minat", "Hobi", "Preferensi", "Gender");
                printf("----------------------------------------------------------------------------------------------------------------------------\n");
                for (int i = 0; i < *numProfiles; i++) {
                    printf("%-20s %-20s %-10d %-15s %-15s %-15s %-15s %-5c\n", profiles[i].full_name, profiles[i].username, profiles[i].age, profiles[i].kepribadian, profiles[i].minat, profiles[i].hobby, profiles[i].activity, profiles[i].gender);
                }
                printf("\nLaporan Aktivitas:\n");
                FILE *reportFile = fopen("matched.txt", "r");
                if (reportFile != NULL) {
                    char firstUsername[50], secondUsername[50], time[50];
                    printf("--------------------------------------------------------------------------------------------------------------------------\n");

                    while (fscanf(reportFile, "%49[^,], %49[^,], %49[^\n]\n", firstUsername, secondUsername, time) != EOF) {
                        printf("%-20s matched with %-20s at %s\n", firstUsername, secondUsername, time);
                    }
                    printf("--------------------------------------------------------------------------------------------------------------------------\n");
                    fclose(reportFile);
                } else {
                    printf("Belum terdapat aktivitas yang dapat dilaporkan.\n");
                }

                printf("Tekan Enter untuk kembali ke menu utama...");
                getchar();
                getchar();
            } else {
                printf("Password kedua salah!\n");
                printf("Tekan Enter untuk kembali ke menu utama...");
                getchar();
                getchar();
            }
        } else {
            userMenu(currentUser, profiles, numProfiles);
        }
    } else {
        printf("Login gagal! Username atau password salah.\n");
        printf("Tekan Enter untuk kembali ke menu utama...");
        getchar();
        getchar(); // To capture the newline character left by scanf
    }
}

void clearScreen() {
    system("cls||clear");
}

void mainMenu(Profile profiles[], int *numProfiles) {
    int choice;
    while (1) {
        clearScreen();
        printf("============================================\n");
        printf("|     Selamat datang di " ORANGE "Sweet n' Sour!" RESET "     |\n");
        printf("============================================\n");
        printf("| 1. Login                                 |\n");
        printf("| 2. Sign Up                               |\n");
        printf("| 3. Program Description                   |\n");
        printf("| 4. Exit                                  |\n");
        printf("============================================\n");
        printf("Pilihan Anda: ");
        scanf("%d", &choice);

        switch (choice) {
        case 1:
            loginMenu(profiles, numProfiles);
            break;
        case 2:
            signUp(profiles, numProfiles);
            break;
        case 3:
            clearScreen();
            printf("=================================================\n");
            printf("|               Program Description             |\n");
            printf("=================================================\n");
            printf("| Program Sweet n' Sour memungkinkan pengguna   |\n| untuk mencari pasangan atau teman berdasarkan |\n| kesamaan minat dan kepribadian mereka.        |\n");
            printf("-------------------------------------------------\n");
            printf("| Sesuai dengan slogan kami...                  |\n| 'Discover Your Perfect Pair~'                 |\n");
            printf("=================================================\n");
            printf("Tekan Enter untuk kembali ke menu utama...");
            getchar();
            getchar(); // To capture the newline character left by scanf
            break;
        case 4:
            printf("Terima kasih telah menggunakan" ORANGE " Sweet n' Sour!!" RESET"\n");
            return;
        default:
            printf("Pilihan tidak valid!\n");
            printf("Tekan Enter untuk kembali ke menu utama...");
            getchar();
            getchar(); // To capture the newline character left by scanf
        }
    }
}

int main() {
    Profile profiles[MAX_PROFILES];
    int numProfiles = 0;

    // Memuat profil pengguna dari file
    printf("\nMemuat profil pengguna dari file...\n");
    if (!loadProfilesFromFile(profiles, &numProfiles, "profiles.txt")) {
        return 1; // Keluar dari program jika terjadi error saat memuat file
    }
    printf("Profil pengguna berhasil dimuat.\n");

    // Menampilkan menu utama
    mainMenu(profiles, &numProfiles);

    // printf("\n%s", profiles[1].full_name);

    return 0;
}