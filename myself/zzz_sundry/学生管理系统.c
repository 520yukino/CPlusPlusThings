#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STUDENTS 30
#define MAX_COURSES 6
#define MAX_NAME_LENGTH 10

typedef struct {
    int id;
    char name[MAX_NAME_LENGTH];
    int scores[MAX_COURSES];
    int totalScore;
    float averageScore;
} Student;

Student students[MAX_STUDENTS];
int numStudents = 0;
int numCourses = 0;

void DisplayStudentData();

void ClearInput()
{
    while (getchar() != '\n')
        continue;
}

//输入学生信息并计算其总分和平均分
void InputStudentData()
{
    printf("Enter the number of students: ");
    scanf("%d", &numStudents);
    ClearInput();
    printf("Enter the number of courses: ");
    scanf("%d", &numCourses);
    ClearInput();
    for (int i = 0; i < numStudents; i++) {
        Student *student = &students[i];
        printf("Enter student %d details:\n", i + 1);
        printf("ID: ");
        scanf("%d", &student->id);
        ClearInput();
        printf("Name: ");
        scanf("%s", student->name);
        ClearInput();
        printf("Enter scores for %d courses:\n", numCourses);
        student->totalScore = 0;
        for (int j = 0; j < numCourses; j++) {
            printf("Course %d: ", j + 1);
            scanf("%d", &student->scores[j]);
            ClearInput();
            student->totalScore += student->scores[j];
        }
        student->averageScore = (float)student->totalScore / numCourses;
    }
}

//计算每门课程的总分和平均分
void CalculateCourseStats()
{
    int totalScores[MAX_COURSES] = {0};
    float averageScores[MAX_COURSES] = {0};
    for (int i = 0; i < numCourses; i++) {
        for (int j = 0; j < numStudents; j++) {
            totalScores[i] += students[j].scores[i];
        }
        averageScores[i] = (float)totalScores[i] / numStudents;
    }
    printf("\nCourse Statistics:\n");
    for (int i = 0; i < numCourses; i++) {
        printf("Course %d: Total Score: %d, Average Score: %.2f\n",
               i + 1, totalScores[i], averageScores[i]);
    }
}

//按每个学生的总分由高到低排出名次表
void SortByTotalScoreDescending()
{
    for (int i = 0; i < numStudents - 1; i++) {
        for (int j = 0; j < numStudents - i - 1; j++) {
            if (students[j].totalScore < students[j + 1].totalScore) {
                Student temp = students[j];
                students[j] = students[j + 1];
                students[j + 1] = temp;
            }
        }
    }
    printf("\nStudents sorted by total score(descending) success\n");
}

//按每个学生的总分由低到高排出名次表
void SortByTotalScoreAscending()
{
    for (int i = 0; i < numStudents - 1; i++) {
        for (int j = 0; j < numStudents - i - 1; j++) {
            if (students[j].totalScore > students[j + 1].totalScore) {
                Student temp = students[j];
                students[j] = students[j + 1];
                students[j + 1] = temp;
            }
        }
    }
    printf("\nStudents sorted by total score(ascending) success\n");
}

//按学号由小到大排出成绩表
void SortById()
{
    for (int i = 0; i < numStudents - 1; i++) {
        for (int j = 0; j < numStudents - i - 1; j++) {
            if (students[j].id > students[j + 1].id) {
                Student temp = students[j];
                students[j] = students[j + 1];
                students[j + 1] = temp;
            }
        }
    }
    printf("\nStudents sorted by ID(ascending) success\n");
}

//按姓名的字典顺序排出成绩表
void SortByNames()
{
    for (int i = 0; i < numStudents - 1; i++) {
        for (int j = 0; j < numStudents - i - 1; j++) {
            if (strcmp(students[j].name, students[j + 1].name) > 0) {
                Student temp = students[j];
                students[j] = students[j + 1];
                students[j + 1] = temp;
            }
        }
    }
    printf("\nStudents sorted by name(ascending) success\n");
}

//按学号查询学生排名及其考试成绩
void SearchById()
{
    int id, flag = 0;
    printf("input ID: ");
    scanf("%d", &id);
    ClearInput();
    for (int i = 0; i < numStudents; i++) {
        if (students[i].id == id) {
            printf("Student with ID %d found:\n", id);
            printf("Name: %s\n", students[i].name);
            for (int j = 0; j < numCourses; j++)
                printf("Course %d: %d\n", j+1, students[i].scores[j]);
            printf("Total Score: %d\n", students[i].totalScore);
            printf("Average Score: %.2f\n", students[i].averageScore);
            printf("Rank: %d\n", i + 1);
            flag = 1;
            break;
        }
    }
    if (!flag)
        printf("Student with ID %d not found.\n", id);
}

//按姓名查询学生排名及其考试成绩
void SearchByName()
{
    char name[MAX_NAME_LENGTH];
    int flag = 0;
    printf("input name: ");
    scanf("%s", &name);
    ClearInput();
    for (int i = 0; i < numStudents; i++) {
        if (strcmp(students[i].name, name) == 0) {
            printf("Student with ID %d found:\n", name);
            printf("Name: %s\n", students[i].name);
            for (int j = 0; j < numCourses; j++)
                printf("Course %d: %d\n", j+1, students[i].scores[j]);
            printf("Total Score: %d\n", students[i].totalScore);
            printf("Average Score: %.2f\n", students[i].averageScore);
            printf("Rank: %d\n", i + 1);
            flag = 1;
            break;
        }
    }
    if (!flag)
        printf("Student with name %s not found.\n", name);
}

//按优秀90~100、良好80~89、中等70~79、及格60~69、不及格0~59这5个类别，对每门课程分别练讨每个类别的人数以及所占的百分比
void CalculateGradeDistribution()
{
    int gradeCount[MAX_COURSES][5] = {0};  // 5 grade categories
    int totalCount[MAX_COURSES] = {0};     // Total number of students per course

    for (int i = 0; i < numStudents; i++) {
        for (int j = 0; j < numCourses; j++) {
            int score = students[i].scores[j];
            int category = 0;

            if (score >= 90) {
                category = 0; // Excellent
            } else if (score >= 80) {
                category = 1; // Good
            } else if (score >= 70) {
                category = 2; // Average
            } else if (score >= 60) {
                category = 3; // Pass
            } else {
                category = 4; // Fail
            }

            gradeCount[j][category]++;
            totalCount[j]++;
        }
    }

    printf("\nGrade Distribution:\n");
    for (int i = 0; i < numCourses; i++) {
        printf("Course %d\n", i + 1);
        printf("Excellent: %d (%.2f%%)\n", gradeCount[i][0], (float)gradeCount[i][0] * 100 / totalCount[i]);
        printf("Good: %d (%.2f%%)\n", gradeCount[i][1], (float)gradeCount[i][1] * 100 / totalCount[i]);
        printf("Average: %d (%.2f%%)\n", gradeCount[i][2], (float)gradeCount[i][2] * 100 / totalCount[i]);
        printf("Pass: %d (%.2f%%)\n", gradeCount[i][3], (float)gradeCount[i][3] * 100 / totalCount[i]);
        printf("Fail: %d (%.2f%%)\n", gradeCount[i][4], (float)gradeCount[i][4] * 100 / totalCount[i]);
    }
}

//输出每个学生的学号、姓名、各科考试成绩，以及每门课程的总分和平均分
void DisplayStudentData()
{
    for (int i = 0; i < numStudents; i++) {
        printf("ID: %d\n", students[i].id);
        printf("Name: %s\n", students[i].name);
        for (int j = 0; j < numCourses; j++) {
            printf("Course %d: %d\n", j+1, students[i].scores[j]);
        }
        printf("Total Score: %d\n", students[i].totalScore);
        printf("Average Score: %.2f\n", students[i].averageScore);
    }
}

//将每个学生的记录信息写入文件
void WriteToFile()
{
    printf("Sure? you will overwrite this file(Y/N): ");
    char c;
    if ((c = getchar()) != 'Y' && c != 'y')
        return;
    FILE *file;
    file = fopen("students_data.txt", "w");
    fprintf(file, "numStudents: %d, numCourses: %d\n", numStudents, numCourses);
    for (int i = 0; i < numStudents; i++) {
        fprintf(file, "ID: %d\n", students[i].id);
        fprintf(file, "Name: %s\n", students[i].name);
        for (int j = 0; j < numCourses; j++) {
            fprintf(file, "Course %d: %d\n", j+1, students[i].scores[j]);
        }
        fprintf(file, "Total Score: %d\n", students[i].totalScore);
        fprintf(file, "Average Score: %.2f\n", students[i].averageScore);
    }
    fclose(file);
    printf("Students record written to file.\n");
}

//从文件中读出每个学生的记录信息并显示
void ReadFromFile()
{
    printf("Sure? you will read data and override yourself data(Y/N): ");
    char c;
    if ((c = getchar()) != 'Y' && c != 'y')
        return;
    FILE *file;
    file = fopen("students_data.txt", "r");
    fscanf(file, "numStudents: %d, numCourses: %d\n", &numStudents, &numCourses);
    for (int i = 0; i < numStudents; i++) {
        fscanf(file, "ID: %d\n", &students[i].id);
        fscanf(file, "Name: %s\n", students[i].name);
        for (int j = 0; j < numCourses; j++) {
            fscanf(file, "Course %*d: %d\n", &students[i].scores[j]);
        }
        fscanf(file, "Total Score: %d\n", &students[i].totalScore);
        fscanf(file, "Average Score: %f\n", &students[i].averageScore);
    }
    fclose(file);
    printf("Students record file read success.\n");
}

//输出菜单并读入用户选择
int Manu()
{
    int choice = -1;
    char c;
    printf("\nStudent Score Management System\n");
    printf("1. Input student data\n");
    printf("2. Calculate course statistics\n");
    printf("3. Sort by total score (descending)\n");
    printf("4. Sort by total score (ascending)\n");
    printf("5. Sort by student ID (ascending)\n");
    printf("6. Sort by names (ascending)\n");
    printf("7. Search by student ID\n");
    printf("8. Search by student name\n");
    printf("9. Calculate grade distribution\n");
    printf("10. Display student data\n");
    printf("11. Write student records to file\n");
    printf("12. Read student records from file\n");
    printf("0. Exit\n");
    printf("Enter your choice: ");
    scanf("%d", &choice);
    ClearInput();
    return choice;
}

int main()
{
    int choice;
    do {
        choice = Manu();
        switch (choice) {
        case 1:
            InputStudentData();
            break;
        case 2:
            CalculateCourseStats();
            break;
        case 3:
            SortByTotalScoreDescending();
            break;
        case 4:
            SortByTotalScoreAscending();
            break;
        case 5:
            SortById();
            break;
        case 6:
            SortByNames();
            break;
        case 7:
            SearchById();
            break;
        case 8:
            SearchByName();
            break;
        case 9:
            CalculateGradeDistribution();
            break;
        case 10:
            DisplayStudentData();
            break;
        case 11:
            WriteToFile();
            break;
        case 12:
            ReadFromFile();
            break;
        case 0:
            printf("Exiting the program.\n");
            break;
        default:
            printf("Invalid choice. Please try again.\n");
        }
    } while (choice != 0);

    return 0;
}
