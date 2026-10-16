/* =========================================================================
   ONLINE EXAMINATION MANAGEMENT SYSTEM
   -------------------------------------------------------------------------
   Console-based C project with three user types: Admin, Teacher, Student.
   Data is stored in plain text files so it survives between runs.

   users.txt      -> id username password role
   exams.txt      -> examid examname subject teacherid duration
   questions.txt  -> qid examid question optionA optionB optionC optionD correctAnswer
   results.txt    -> resultid studentid examid score total

   IMPORTANT: fields are separated by spaces, so none of them can contain
   a space. When you're asked for an exam name, a question, or an answer
   option, use an underscore instead of a space, e.g. "General_Knowledge"
   or "What_is_2_plus_2".

   Default admin login: admin / admin123 (created automatically on first run)

   Compile:  gcc exam_system.c -o exam_system
   Run:      ./exam_system
   ========================================================================= */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ---------------------------- Structures --------------------------- */

struct User {
    int id;
    char username[50];
    char password[50];
    char role[10];   /* admin / teacher / student */
};

struct Exam {
    int examid;
    char examname[100];
    char subject[50];
    int teacherid;
    int duration;    /* minutes, just for display, not enforced */
};

struct Question {
    int qid;
    int examid;
    char questiontext[200];
    char optionA[100];
    char optionB[100];
    char optionC[100];
    char optionD[100];
    char correctAnswer;   /* A / B / C / D */
};

struct Result {
    int resultid;
    int studentid;
    int examid;
    int score;
    int total;
};

/* ------------------------------ Function Prototypes ------------------------------ */

int getNextId(char *filename);
int readUserLine(FILE *fp, struct User *u);
int readExamLine(FILE *fp, struct Exam *e);
int readQuestionLine(FILE *fp, struct Question *q);
int readResultLine(FILE *fp, struct Result *r);
void getUsernameById(int id, char *result);
void getExamNameById(int id, char *result);
void createDefaultAdmin(void);

int login(char *role, struct User *loggedInUser);
void registerUser(char *role);

void adminMenu(struct User admin);
void viewAllUsers(void);
void viewAllExams(void);
void viewSystemStats(void);
void searchUserByUsername(void);

void teacherMenu(struct User teacher);
void createExam(struct User teacher);
void addQuestions(struct User teacher);
void viewMyExams(struct User teacher);
void viewExamQuestions(struct User teacher);
void examStatistics(struct User teacher);

void studentMenu(struct User student);
void takeExam(struct User student);
void viewMyResults(struct User student);
void viewAvailableExamsForStudent(struct User student);
void viewMyProfile(struct User student);

void registerMenu(void);
void loginMenu(void);
void mainMenu(void);

/* =====================================================================
   HELPER FUNCTIONS
   ===================================================================== */

/* Finds the largest ID already used in a file and returns the next one. */
int getNextId(char *filename) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) return 1;

    int biggest = 0;
    int id;
    while (fscanf(fp, "%d", &id) == 1) {
        if (id > biggest) biggest = id;
        while (fgetc(fp) != '\n' && !feof(fp));   /* skip the rest of this line */
    }

    fclose(fp);
    return biggest + 1;
}

/* Each of these reads ONE record from the file directly into a struct.
   fscanf() reads values separated by spaces (or newlines) and stops at
   the next space -- that's why none of our text fields can contain a
   space. It returns how many values it managed to read, so we compare
   that to how many fields we expected. */

int readUserLine(FILE *fp, struct User *u) {
    return fscanf(fp, "%d %49s %49s %9s",
                  &u->id, u->username, u->password, u->role) == 4;
}

int readExamLine(FILE *fp, struct Exam *e) {
    return fscanf(fp, "%d %99s %49s %d %d",
                  &e->examid, e->examname, e->subject,
                  &e->teacherid, &e->duration) == 5;
}

int readQuestionLine(FILE *fp, struct Question *q) {
    return fscanf(fp, "%d %d %199s %99s %99s %99s %99s %c",
                  &q->qid, &q->examid, q->questiontext,
                  q->optionA, q->optionB, q->optionC, q->optionD,
                  &q->correctAnswer) == 8;
}

int readResultLine(FILE *fp, struct Result *r) {
    return fscanf(fp, "%d %d %d %d %d",
                  &r->resultid, &r->studentid, &r->examid,
                  &r->score, &r->total) == 5;
}

void getUsernameById(int id, char *result) {
    strcpy(result, "Unknown");

    FILE *fp = fopen("users.txt", "r");
    if (fp == NULL) return;

    struct User u;
    while (readUserLine(fp, &u)) {
        if (u.id == id) {
            strcpy(result, u.username);
            break;
        }
    }
    fclose(fp);
}

void getExamNameById(int id, char *result) {
    strcpy(result, "Unknown");

    FILE *fp = fopen("exams.txt", "r");
    if (fp == NULL) return;

    struct Exam e;
    while (readExamLine(fp, &e)) {
        if (e.examid == id) {
            strcpy(result, e.examname);
            break;
        }
    }
    fclose(fp);
}

/* Runs once at startup so there's always an admin account to log into. */
void createDefaultAdmin(void) {
    FILE *fp = fopen("users.txt", "r");
    int adminExists = 0;

    if (fp != NULL) {
        struct User u;
        while (readUserLine(fp, &u)) {
            if (strcmp(u.role, "admin") == 0) {
                adminExists = 1;
                break;
            }
        }
        fclose(fp);
    }

    if (adminExists == 0) {
        fp = fopen("users.txt", "a");
        if (fp == NULL) {
            printf("Error: could not create the default admin account.\n");
            return;
        }
        fprintf(fp, "%d admin admin123 admin\n", getNextId("users.txt"));
        fclose(fp);
    }
}

/* =====================================================================
   LOGIN / REGISTER FUNCTIONS
   ===================================================================== */

int login(char *role, struct User *loggedInUser) {
    char username[50], password[50];
    printf("Username: ");
    scanf(" %49s", username);
    printf("Password: ");
    scanf(" %49s", password);

    FILE *fp = fopen("users.txt", "r");
    if (fp == NULL) return 0;

    struct User u;
    while (readUserLine(fp, &u)) {
        if (strcmp(u.username, username) == 0 &&
            strcmp(u.password, password) == 0 &&
            strcmp(u.role, role) == 0) {
            *loggedInUser = u;
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

void registerUser(char *role) {
    char username[50], password[50];
    printf("Choose a username (no spaces): ");
    scanf(" %49s", username);

    FILE *fp = fopen("users.txt", "r");
    if (fp != NULL) {
        struct User u;
        while (readUserLine(fp, &u)) {
            if (strcmp(u.username, username) == 0) {
                printf("Sorry, that username is already taken.\n");
                fclose(fp);
                return;
            }
        }
        fclose(fp);
    }

    printf("Choose a password (no spaces): ");
    scanf(" %49s", password);

    int newId = getNextId("users.txt");
    fp = fopen("users.txt", "a");
    if (fp == NULL) {
        printf("Error: could not save your account. Please try again.\n");
        return;
    }
    fprintf(fp, "%d %s %s %s\n", newId, username, password, role);
    fclose(fp);

    printf("Registration successful! Your user ID is %d.\n", newId);
}

/* =====================================================================
   ADMIN FUNCTIONS
   ===================================================================== */

void adminMenu(struct User admin) {
    int choice;
    do {
        printf("\n--- ADMIN MENU (%s) ---\n", admin.username);
        printf("1. View All Users\n");
        printf("2. View All Exams\n");
        printf("3. View System Statistics\n");
        printf("4. Search User by Username\n");
        printf("5. Logout\n");
        printf("Enter choice: ");
        scanf("%d", &choice);

        if (choice == 1) viewAllUsers();
        else if (choice == 2) viewAllExams();
        else if (choice == 3) viewSystemStats();
        else if (choice == 4) searchUserByUsername();
        else if (choice == 5) printf("Logging out...\n");
        else printf("Invalid choice. Try again.\n");

    } while (choice != 5);
}

void viewAllUsers(void) {
    FILE *fp = fopen("users.txt", "r");
    if (fp == NULL) { printf("No users found.\n"); return; }

    struct User u;
    printf("\n%-5s %-20s %-10s\n", "ID", "Username", "Role");
    printf("-----------------------------------\n");
    while (readUserLine(fp, &u)) {
        printf("%-5d %-20s %-10s\n", u.id, u.username, u.role);
    }
    fclose(fp);
}

void viewAllExams(void) {
    FILE *fp = fopen("exams.txt", "r");
    if (fp == NULL) { printf("No exams found.\n"); return; }

    struct Exam e;
    printf("\n%-5s %-20s %-15s %-10s\n", "ID", "Exam Name", "Subject", "Duration");
    printf("------------------------------------------------------\n");
    while (readExamLine(fp, &e)) {
        printf("%-5d %-20s %-15s %-10d\n",
               e.examid, e.examname, e.subject, e.duration);
    }
    fclose(fp);
}

void viewSystemStats(void) {
    int totalUsers = 0, totalAdmins = 0, totalTeachers = 0, totalStudents = 0;
    FILE *fp = fopen("users.txt", "r");
    struct User u;
    while (fp != NULL && readUserLine(fp, &u)) {
        totalUsers++;
        if (strcmp(u.role, "admin") == 0) totalAdmins++;
        else if (strcmp(u.role, "teacher") == 0) totalTeachers++;
        else if (strcmp(u.role, "student") == 0) totalStudents++;
    }
    if (fp != NULL) fclose(fp);

    int totalExams = 0;
    fp = fopen("exams.txt", "r");
    struct Exam e;
    while (fp != NULL && readExamLine(fp, &e)) totalExams++;
    if (fp != NULL) fclose(fp);

    int totalQuestions = 0;
    fp = fopen("questions.txt", "r");
    struct Question q;
    while (fp != NULL && readQuestionLine(fp, &q)) totalQuestions++;
    if (fp != NULL) fclose(fp);

    int totalAttempts = 0;
    fp = fopen("results.txt", "r");
    struct Result r;
    while (fp != NULL && readResultLine(fp, &r)) totalAttempts++;
    if (fp != NULL) fclose(fp);

    printf("\n--- SYSTEM STATISTICS ---\n");
    printf("Total users      : %d\n", totalUsers);
    printf("  Admins         : %d\n", totalAdmins);
    printf("  Teachers       : %d\n", totalTeachers);
    printf("  Students       : %d\n", totalStudents);
    printf("Total exams      : %d\n", totalExams);
    printf("Total questions  : %d\n", totalQuestions);
    printf("Total attempts   : %d\n", totalAttempts);
}

void searchUserByUsername(void) {
    char searchName[50];
    printf("Enter username to search: ");
    scanf(" %49s", searchName);

    FILE *fp = fopen("users.txt", "r");
    if (fp == NULL) { printf("No users found.\n"); return; }

    struct User u;
    int found = 0;
    while (readUserLine(fp, &u)) {
        if (strcmp(u.username, searchName) == 0) {
            printf("\nUser found:\n");
            printf("ID       : %d\n", u.id);
            printf("Username : %s\n", u.username);
            printf("Role     : %s\n", u.role);
            found = 1;
            break;
        }
    }
    fclose(fp);

    if (!found) printf("No user found with that username.\n");
}

/* =====================================================================
   TEACHER FUNCTIONS
   ===================================================================== */

void teacherMenu(struct User teacher) {
    int choice;
    do {
        printf("\n--- TEACHER MENU (%s) ---\n", teacher.username);
        printf("1. Create New Exam\n");
        printf("2. Add Questions to My Exam\n");
        printf("3. View My Exams\n");
        printf("4. View Questions in My Exam\n");
        printf("5. View Exam Statistics\n");
        printf("6. Logout\n");
        printf("Enter choice: ");
        scanf("%d", &choice);

        if (choice == 1) createExam(teacher);
        else if (choice == 2) addQuestions(teacher);
        else if (choice == 3) viewMyExams(teacher);
        else if (choice == 4) viewExamQuestions(teacher);
        else if (choice == 5) examStatistics(teacher);
        else if (choice == 6) printf("Logging out...\n");
        else printf("Invalid choice. Try again.\n");

    } while (choice != 6);
}

void createExam(struct User teacher) {
    struct Exam e;
    e.examid = getNextId("exams.txt");
    e.teacherid = teacher.id;

    printf("Enter exam name (no spaces, use _ instead): ");
    scanf(" %99s", e.examname);
    printf("Enter subject (no spaces): ");
    scanf(" %49s", e.subject);
    printf("Enter time limit in minutes: ");
    scanf("%d", &e.duration);

    FILE *fp = fopen("exams.txt", "a");
    if (fp == NULL) {
        printf("Error: could not save the exam. Please try again.\n");
        return;
    }
    fprintf(fp, "%d %s %s %d %d\n",
            e.examid, e.examname, e.subject, e.teacherid, e.duration);
    fclose(fp);

    printf("Exam created! Its ID is %d. Now add questions to it.\n", e.examid);
}

void addQuestions(struct User teacher) {
    int examid;
    printf("Enter the Exam ID: ");
    scanf("%d", &examid);

    /* only the teacher who owns this exam can add questions to it */
    FILE *fp = fopen("exams.txt", "r");
    struct Exam e;
    int found = 0;
    while (fp != NULL && readExamLine(fp, &e)) {
        if (e.examid == examid && e.teacherid == teacher.id) {
            found = 1;
            break;
        }
    }
    if (fp != NULL) fclose(fp);

    if (!found) {
        printf("That exam does not exist, or it is not yours.\n");
        return;
    }

    int count;
    printf("How many questions do you want to add? ");
    scanf("%d", &count);

    fp = fopen("questions.txt", "a");
    if (fp == NULL) {
        printf("Error: could not open questions.txt to save questions.\n");
        return;
    }

    for (int i = 0; i < count; i++) {
        struct Question q;
        q.qid = getNextId("questions.txt");
        q.examid = examid;

        printf("\nQuestion %d (no spaces, use _ instead):\n", i + 1);
        printf("  Question text: ");
        scanf(" %199s", q.questiontext);
        printf("  Option A: ");
        scanf(" %99s", q.optionA);
        printf("  Option B: ");
        scanf(" %99s", q.optionB);
        printf("  Option C: ");
        scanf(" %99s", q.optionC);
        printf("  Option D: ");
        scanf(" %99s", q.optionD);

        do {
            printf("  Correct answer (A/B/C/D): ");
            scanf(" %c", &q.correctAnswer);
            q.correctAnswer = toupper(q.correctAnswer);
        } while (q.correctAnswer != 'A' && q.correctAnswer != 'B' &&
                 q.correctAnswer != 'C' && q.correctAnswer != 'D');

        fprintf(fp, "%d %d %s %s %s %s %s %c\n",
                q.qid, q.examid, q.questiontext,
                q.optionA, q.optionB, q.optionC, q.optionD, q.correctAnswer);
    }
    fclose(fp);
    printf("%d question(s) added.\n", count);
}

void viewMyExams(struct User teacher) {
    FILE *fp = fopen("exams.txt", "r");
    if (fp == NULL) { printf("No exams found.\n"); return; }

    struct Exam e;
    int any = 0;
    printf("\n%-5s %-20s %-15s\n", "ID", "Exam Name", "Subject");
    printf("--------------------------------------------\n");
    while (readExamLine(fp, &e)) {
        if (e.teacherid == teacher.id) {
            printf("%-5d %-20s %-15s\n", e.examid, e.examname, e.subject);
            any = 1;
        }
    }
    if (!any) printf("You have not created any exams yet.\n");
    fclose(fp);
}

void viewExamQuestions(struct User teacher) {
    int examid;
    printf("Enter Exam ID: ");
    scanf("%d", &examid);

    FILE *fp = fopen("exams.txt", "r");
    struct Exam e;
    int found = 0;
    while (fp != NULL && readExamLine(fp, &e)) {
        if (e.examid == examid && e.teacherid == teacher.id) {
            found = 1;
            break;
        }
    }
    if (fp != NULL) fclose(fp);

    if (!found) {
        printf("That exam does not exist, or it is not yours.\n");
        return;
    }

    fp = fopen("questions.txt", "r");
    if (fp == NULL) { printf("No questions found.\n"); return; }

    struct Question q;
    int qnum = 0;
    while (readQuestionLine(fp, &q)) {
        if (q.examid == examid) {
            qnum++;
            printf("\nQ%d. %s\n", qnum, q.questiontext);
            printf("  A) %s\n  B) %s\n  C) %s\n  D) %s\n",
                   q.optionA, q.optionB, q.optionC, q.optionD);
            printf("  Correct answer: %c\n", q.correctAnswer);
        }
    }
    fclose(fp);

    if (qnum == 0) printf("This exam does not have any questions yet.\n");
}

void examStatistics(struct User teacher) {
    int examid;
    printf("Enter Exam ID: ");
    scanf("%d", &examid);

    FILE *fp = fopen("exams.txt", "r");
    struct Exam e;
    int found = 0;
    while (fp != NULL && readExamLine(fp, &e)) {
        if (e.examid == examid && e.teacherid == teacher.id) {
            found = 1;
            break;
        }
    }
    if (fp != NULL) fclose(fp);

    if (!found) {
        printf("That exam does not exist, or it is not yours.\n");
        return;
    }

    fp = fopen("results.txt", "r");
    if (fp == NULL) { printf("No students have attempted this exam yet.\n"); return; }

    struct Result r;
    int attempts = 0;
    int highest = -1;
    int scoreSum = 0;
    int totalPossible = 0;

    while (readResultLine(fp, &r)) {
        if (r.examid == examid) {
            attempts++;
            scoreSum += r.score;
            totalPossible = r.total;
            if (r.score > highest) highest = r.score;
        }
    }
    fclose(fp);

    if (attempts == 0) {
        printf("No students have attempted this exam yet.\n");
        return;
    }

    float average = (float) scoreSum / attempts;
    printf("\n--- STATISTICS FOR EXAM %d ---\n", examid);
    printf("Number of attempts : %d\n", attempts);
    printf("Highest score      : %d/%d\n", highest, totalPossible);
    printf("Average score      : %.1f/%d\n", average, totalPossible);
}

/* =====================================================================
   STUDENT FUNCTIONS
   ===================================================================== */

void studentMenu(struct User student) {
    int choice;
    do {
        printf("\n--- STUDENT MENU (%s) ---\n", student.username);
        printf("1. View Available Exams\n");
        printf("2. Take Exam\n");
        printf("3. View My Results\n");
        printf("4. My Profile\n");
        printf("5. Logout\n");
        printf("Enter choice: ");
        scanf("%d", &choice);

        if (choice == 1) viewAvailableExamsForStudent(student);
        else if (choice == 2) takeExam(student);
        else if (choice == 3) viewMyResults(student);
        else if (choice == 4) viewMyProfile(student);
        else if (choice == 5) printf("Logging out...\n");
        else printf("Invalid choice. Try again.\n");

    } while (choice != 5);
}

void takeExam(struct User student) {
    int examid;
    printf("Enter Exam ID to take: ");
    scanf("%d", &examid);

    FILE *fp = fopen("exams.txt", "r");
    struct Exam e;
    int found = 0;
    while (fp != NULL && readExamLine(fp, &e)) {
        if (e.examid == examid) {
            found = 1;
            break;
        }
    }
    if (fp != NULL) fclose(fp);

    if (!found) { printf("Exam not found.\n"); return; }

    fp = fopen("results.txt", "r");
    struct Result r;
    while (fp != NULL && readResultLine(fp, &r)) {
        if (r.studentid == student.id && r.examid == examid) {
            printf("You have already taken this exam. Your score was %d/%d.\n", r.score, r.total);
            fclose(fp);
            return;
        }
    }
    if (fp != NULL) fclose(fp);

    printf("\nStarting exam: %s (%s)\n", e.examname, e.subject);
    printf("Time limit: %d minute(s)\n", e.duration);

    fp = fopen("questions.txt", "r");
    if (fp == NULL) {
        printf("This exam does not have any questions yet.\n");
        return;
    }

    struct Question q;
    int score = 0;
    int total = 0;
    int qnum = 0;

    while (readQuestionLine(fp, &q)) {
        if (q.examid != examid) continue;

        qnum++;
        total++;

        printf("\nQ%d. %s\n", qnum, q.questiontext);
        printf("  A) %s\n", q.optionA);
        printf("  B) %s\n", q.optionB);
        printf("  C) %s\n", q.optionC);
        printf("  D) %s\n", q.optionD);

        char answer;
        do {
            printf("Your answer (A/B/C/D): ");
            scanf(" %c", &answer);
            answer = toupper(answer);
        } while (answer != 'A' && answer != 'B' && answer != 'C' && answer != 'D');

        if (answer == q.correctAnswer) {
            score++;
        }
    }
    fclose(fp);

    if (total == 0) {
        printf("This exam does not have any questions yet.\n");
        return;
    }

    float percent = (score * 100.0) / total;
    printf("\nExam finished! Your score: %d/%d (%.1f%%)\n", score, total, percent);
    if (percent >= 50)
        printf("Result: PASS\n");
    else
        printf("Result: FAIL\n");

    int resultId = getNextId("results.txt");
    fp = fopen("results.txt", "a");
    if (fp == NULL) {
        printf("Error: could not save your result. Please tell your teacher.\n");
        return;
    }
    fprintf(fp, "%d %d %d %d %d\n", resultId, student.id, examid, score, total);
    fclose(fp);
}

void viewMyResults(struct User student) {
    FILE *fp = fopen("results.txt", "r");
    if (fp == NULL) { printf("No results found.\n"); return; }

    struct Result r;
    char examName[100];
    int any = 0;

    printf("\n%-20s %-10s %-8s\n", "Exam", "Score", "Percent");
    printf("---------------------------------------\n");
    while (readResultLine(fp, &r)) {
        if (r.studentid == student.id) {
            getExamNameById(r.examid, examName);
            float percent = (r.score * 100.0) / r.total;
            printf("%-20s %d/%-8d %.1f%%\n", examName, r.score, r.total, percent);
            any = 1;
        }
    }
    if (!any) printf("You have not taken any exams yet.\n");
    fclose(fp);
}

/* Only shows exams this student hasn't taken yet -- for each exam we
   check results.txt for a matching attempt before printing it. */
void viewAvailableExamsForStudent(struct User student) {
    FILE *examFile = fopen("exams.txt", "r");
    if (examFile == NULL) { printf("No exams found.\n"); return; }

    struct Exam e;
    int any = 0;

    printf("\n%-5s %-20s %-15s\n", "ID", "Exam Name", "Subject");
    printf("--------------------------------------------\n");

    while (readExamLine(examFile, &e)) {
        int alreadyTaken = 0;
        FILE *resultFile = fopen("results.txt", "r");
        struct Result r;
        while (resultFile != NULL && readResultLine(resultFile, &r)) {
            if (r.studentid == student.id && r.examid == e.examid) {
                alreadyTaken = 1;
                break;
            }
        }
        if (resultFile != NULL) fclose(resultFile);

        if (!alreadyTaken) {
            printf("%-5d %-20s %-15s\n", e.examid, e.examname, e.subject);
            any = 1;
        }
    }
    fclose(examFile);

    if (!any) printf("No new exams available right now.\n");
}

void viewMyProfile(struct User student) {
    printf("\n--- MY PROFILE ---\n");
    printf("User ID  : %d\n", student.id);
    printf("Username : %s\n", student.username);
    printf("Role     : %s\n", student.role);

    FILE *fp = fopen("results.txt", "r");
    int examsTaken = 0;
    if (fp != NULL) {
        struct Result r;
        while (readResultLine(fp, &r)) {
            if (r.studentid == student.id) examsTaken++;
        }
        fclose(fp);
    }
    printf("Exams taken so far: %d\n", examsTaken);
}

/* =====================================================================
   TOP LEVEL MENUS
   ===================================================================== */

void registerMenu(void) {
    int choice;
    printf("Register as:\n1. Admin (not available)\n2. Teacher\n3. Student\nEnter choice: ");
    scanf("%d", &choice);

    if (choice == 2) registerUser("teacher");
    else if (choice == 3) registerUser("student");
    else if (choice == 1) printf("Admin accounts cannot be self-registered.\n");
    else printf("Invalid choice.\n");
}

void loginMenu(void) {
    int choice;
    struct User loggedInUser;

    printf("Login as:\n1. Admin\n2. Teacher\n3. Student\nEnter choice: ");
    scanf("%d", &choice);

    char role[10];
    if (choice == 1) strcpy(role, "admin");
    else if (choice == 2) strcpy(role, "teacher");
    else if (choice == 3) strcpy(role, "student");
    else { printf("Invalid choice.\n"); return; }

    if (login(role, &loggedInUser)) {
        printf("Login successful! Welcome, %s.\n", loggedInUser.username);

        if (choice == 1) adminMenu(loggedInUser);
        else if (choice == 2) teacherMenu(loggedInUser);
        else studentMenu(loggedInUser);
    } else {
        printf("Wrong username or password.\n");
    }
}

void mainMenu(void) {
    int choice;
    do {
        printf("\n===== ONLINE EXAMINATION MANAGEMENT SYSTEM =====\n");
        printf("1. Register\n");
        printf("2. Login\n");
        printf("3. Exit\n");
        printf("Enter choice: ");

        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n');   /* clear bad input so we don't get stuck */
            choice = 0;
        }

        if (choice == 1) registerMenu();
        else if (choice == 2) loginMenu();
        else if (choice == 3) printf("Thank you for using the system. Goodbye!\n");
        else printf("Invalid choice. Try again.\n");

    } while (choice != 3);
}

int main(void) {
    createDefaultAdmin();
    mainMenu();
    return 0;
}
