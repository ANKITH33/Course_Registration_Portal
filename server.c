#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>


typedef struct {
    int id;
    char name[50];
    char password[50];
    int is_active;
} Student;

typedef struct {
    int id;
    char name[50];
    char password[50];
} Faculty;

typedef struct {
    int id;
    char name[50];
    char faculty_name[50];
    int faculty_id;
    int seats;
    int enrolled_students[100];
    int enrolled_count;
} Course;

Student students[100];
Faculty faculty[100];
Course courses[50];
int student_count = 0;
int faculty_count = 0;
int course_count = 0;

sem_t student_sem, faculty_sem, course_sem;

void load_data();
void save_data();
void *handle_client(void *arg);
void admin_menu(int client_socket);
void faculty_menu(int client_socket, int faculty_id);
void student_menu(int client_socket, int student_id);

void add_student(int client_socket) {
    Student new_student;
    char buffer[1024] = {0};
    int bytes_read;
    
    send(client_socket, "Enter student name: ", 20, 0);
    bytes_read = read(client_socket, buffer, 49);
    if (bytes_read <= 0) return;
    buffer[bytes_read] = '\0';
    strncpy(new_student.name, buffer, 49);
    new_student.name[49] = '\0';
    memset(buffer, 0, 1024);
    
    send(client_socket, "Enter student password: ", 24, 0);
    bytes_read = read(client_socket, buffer, 49);
    if (bytes_read <= 0) return;
    buffer[bytes_read] = '\0';
    strncpy(new_student.password, buffer, 49);
    new_student.password[49] = '\0';
    
    sem_wait(&student_sem);
    new_student.id = student_count + 1;
    new_student.is_active = 1;
    students[student_count++] = new_student;
    sem_post(&student_sem);
    
    send(client_socket, "Student added successfully!\n", 27, 0);
    save_data();
}

void view_student_details(int client_socket) {
    char buffer[1024] = {0};
    int id;
    
    send(client_socket, "Enter student ID: ", 18, 0);
    read(client_socket, buffer, 1024);
    id = atoi(buffer);
    memset(buffer, 0, 1024);
    
    sem_wait(&student_sem);
    for (int i = 0; i < student_count; i++) {
        if (students[i].id == id) {
            sprintf(buffer, "ID: %d\nName: %s\nStatus: %s\n", 
                   students[i].id, students[i].name, 
                   students[i].is_active ? "Active" : "Blocked");
            send(client_socket, buffer, strlen(buffer), 0);
            sem_post(&student_sem);
            return;
        }
    }
    sem_post(&student_sem);
    
    send(client_socket, "Student not found!\n", 19, 0);
}

void add_faculty(int client_socket) {
    Faculty new_faculty;
    char buffer[1024] = {0};
    
    send(client_socket, "Enter faculty name: ", 20, 0);
    read(client_socket, new_faculty.name, 50);
    
    send(client_socket, "Enter faculty password: ", 24, 0);
    read(client_socket, new_faculty.password, 50);
    
    sem_wait(&faculty_sem);
    new_faculty.id = faculty_count + 1;
    faculty[faculty_count++] = new_faculty;
    sem_post(&faculty_sem);
    
    send(client_socket, "Faculty added successfully!\n", 27, 0);
    save_data();
}

void view_faculty_details(int client_socket) {
    char buffer[1024] = {0};
    int id;
    
    send(client_socket, "Enter faculty ID: ", 18, 0);
    read(client_socket, buffer, 1024);
    id = atoi(buffer);
    memset(buffer, 0, 1024);
    
    sem_wait(&faculty_sem);
    for (int i = 0; i < faculty_count; i++) {
        if (faculty[i].id == id) {
            sprintf(buffer, "ID: %d\nName: %s\n", faculty[i].id, faculty[i].name);
            send(client_socket, buffer, strlen(buffer), 0);
            sem_post(&faculty_sem);
            return;
        }
    }
    sem_post(&faculty_sem);
    
    send(client_socket, "Faculty not found!\n", 19, 0);
}

void activate_student(int client_socket) {
    char buffer[1024] = {0};
    int id;
    
    send(client_socket, "Enter student ID to activate: ", 29, 0);
    read(client_socket, buffer, 1024);
    id = atoi(buffer);
    memset(buffer, 0, 1024);
    
    sem_wait(&student_sem);
    for (int i = 0; i < student_count; i++) {
        if (students[i].id == id) {
            students[i].is_active = 1;
            send(client_socket, "Student activated successfully!\n", 31, 0);
            sem_post(&student_sem);
            save_data();
            return;
        }
    }
    sem_post(&student_sem);
    
    send(client_socket, "Student not found!\n", 19, 0);
}

void block_student(int client_socket) {
    char buffer[1024] = {0};
    int id;
    
    send(client_socket, "Enter student ID to block: ", 27, 0);
    read(client_socket, buffer, 1024);
    id = atoi(buffer);
    memset(buffer, 0, 1024);
    
    sem_wait(&student_sem);
    for (int i = 0; i < student_count; i++) {
        if (students[i].id == id) {
            students[i].is_active = 0;
            send(client_socket, "Student blocked successfully!\n", 30, 0);
            sem_post(&student_sem);
            save_data();
            return;
        }
    }
    sem_post(&student_sem);
    
    send(client_socket, "Student not found!\n", 19, 0);
}

void modify_student_details(int client_socket) {
    char buffer[1024] = {0};
    int id, choice;
    
    send(client_socket, "Enter student ID to modify: ", 28, 0);
    read(client_socket, buffer, 1024);
    id = atoi(buffer);
    memset(buffer, 0, 1024);
    
    sem_wait(&student_sem);
    for (int i = 0; i < student_count; i++) {
        if (students[i].id == id) {
            char *menu = "What do you want to modify?\n"
                         "1. Name\n"
                         "2. Password\n"
                         "Enter your choice: ";
            send(client_socket, menu, strlen(menu), 0);
            
            read(client_socket, buffer, 1024);
            choice = atoi(buffer);
            memset(buffer, 0, 1024);
            
            switch (choice) {
                case 1:
                    send(client_socket, "Enter new name: ", 16, 0);
                    read(client_socket, students[i].name, 50);
                    break;
                case 2:
                    send(client_socket, "Enter new password: ", 20, 0);
                    read(client_socket, students[i].password, 50);
                    break;
                default:
                    send(client_socket, "Invalid choice!\n", 16, 0);
                    sem_post(&student_sem);
                    return;
            }
            
            send(client_socket, "Student details updated successfully!\n", 37, 0);
            sem_post(&student_sem);
            save_data();
            return;
        }
    }
    sem_post(&student_sem);
    
    send(client_socket, "Student not found!\n", 19, 0);
}

void modify_faculty_details(int client_socket) {
    char buffer[1024] = {0};
    int id, choice;
    
    send(client_socket, "Enter faculty ID to modify: ", 28, 0);
    read(client_socket, buffer, 1024);
    id = atoi(buffer);
    memset(buffer, 0, 1024);
    
    sem_wait(&faculty_sem);
    for (int i = 0; i < faculty_count; i++) {
        if (faculty[i].id == id) {
            char *menu = "What do you want to modify?\n"
                         "1. Name\n"
                         "2. Password\n"
                         "Enter your choice: ";
            send(client_socket, menu, strlen(menu), 0);
            
            read(client_socket, buffer, 1024);
            choice = atoi(buffer);
            memset(buffer, 0, 1024);
            
            switch (choice) {
                case 1:
                    send(client_socket, "Enter new name: ", 16, 0);
                    read(client_socket, faculty[i].name, 50);
                    break;
                case 2:
                    send(client_socket, "Enter new password: ", 20, 0);
                    read(client_socket, faculty[i].password, 50);
                    break;
                default:
                    send(client_socket, "Invalid choice!\n", 16, 0);
                    sem_post(&faculty_sem);
                    return;
            }
            
            send(client_socket, "Faculty details updated successfully!\n", 37, 0);
            sem_post(&faculty_sem);
            save_data();
            return;
        }
    }
    sem_post(&faculty_sem);
    
    send(client_socket, "Faculty not found!\n", 19, 0);
}

void view_offering_courses(int client_socket, int faculty_id) {
    char buffer[1024] = {0};
    int found = 0;
    
    sem_wait(&course_sem);
    for (int i = 0; i < course_count; i++) {
        if (courses[i].faculty_id == faculty_id) {
            sprintf(buffer, "Course ID: %d\nName: %s\nFaculty: %s\nSeats: %d\nEnrolled: %d\n\n",
                   courses[i].id, courses[i].name, courses[i].faculty_name,
                   courses[i].seats, courses[i].enrolled_count);
            send(client_socket, buffer, strlen(buffer), 0);
            found = 1;
            memset(buffer, 0, 1024);
        }
    }
    sem_post(&course_sem);
    
    if (!found) {
        send(client_socket, "No courses found for this faculty!\n", 34, 0);
    }
}

void add_course(int client_socket, int faculty_id) {
    Course new_course;
    char buffer[1024] = {0};
    
    send(client_socket, "Enter course name: ", 19, 0);
    read(client_socket, new_course.name, 50);
    
    send(client_socket, "Enter number of seats: ", 23, 0);
    read(client_socket, buffer, 1024);
    new_course.seats = atoi(buffer);
    memset(buffer, 0, 1024);
    
    sem_wait(&faculty_sem);
    for (int i = 0; i < faculty_count; i++) {
        if (faculty[i].id == faculty_id) {
            strcpy(new_course.faculty_name, faculty[i].name);
            break;
        }
    }
    sem_post(&faculty_sem);
    
    sem_wait(&course_sem);
    new_course.id = course_count + 1;
    new_course.faculty_id = faculty_id;
    new_course.enrolled_count = 0;
    memset(new_course.enrolled_students, 0, sizeof(new_course.enrolled_students));
    
    courses[course_count++] = new_course;
    sem_post(&course_sem);
    
    send(client_socket, "Course added successfully!\n", 27, 0);
    save_data();
}

void remove_course(int client_socket, int faculty_id) {
    char buffer[1024] = {0};
    int id, found = 0;
    
    send(client_socket, "Enter course ID to remove: ", 27, 0);
    read(client_socket, buffer, 1024);
    id = atoi(buffer);
    memset(buffer, 0, 1024);
    
    sem_wait(&course_sem);
    for (int i = 0; i < course_count; i++) {
        if (courses[i].id == id && courses[i].faculty_id == faculty_id) {
            for (int j = i; j < course_count - 1; j++) {
                courses[j] = courses[j + 1];
            }
            course_count--;
            found = 1;
            break;
        }
    }
    sem_post(&course_sem);
    
    if (found) {
        send(client_socket, "Course removed successfully!\n", 28, 0);
        save_data();
    } else {
        send(client_socket, "Course not found or not owned by you!\n", 36, 0);
    }
}

void update_course_details(int client_socket, int faculty_id) {
    char buffer[1024] = {0};
    int id, choice;
    
    send(client_socket, "Enter course ID to update: ", 27, 0);
    read(client_socket, buffer, 1024);
    id = atoi(buffer);
    memset(buffer, 0, 1024);
    
    sem_wait(&course_sem);
    for (int i = 0; i < course_count; i++) {
        if (courses[i].id == id && courses[i].faculty_id == faculty_id) {
            char *menu = "What do you want to update?\n"
                         "1. Course name\n"
                         "2. Number of seats\n"
                         "Enter your choice: ";
            send(client_socket, menu, strlen(menu), 0);
            
            read(client_socket, buffer, 1024);
            choice = atoi(buffer);
            memset(buffer, 0, 1024);
            
            switch (choice) {
                case 1:
                    send(client_socket, "Enter new course name: ", 22, 0);
                    read(client_socket, courses[i].name, 50);
                    break;
                case 2:
                    send(client_socket, "Enter new number of seats: ", 27, 0);
                    read(client_socket, buffer, 1024);
                    courses[i].seats = atoi(buffer);
                    memset(buffer, 0, 1024);
                    break;
                default:
                    send(client_socket, "Invalid choice!\n", 16, 0);
                    sem_post(&course_sem);
                    return;
            }
            
            send(client_socket, "Course updated successfully!\n", 29, 0);
            sem_post(&course_sem);
            save_data();
            return;
        }
    }
    sem_post(&course_sem);
    
    send(client_socket, "Course not found or not owned by you!\n", 36, 0);
}

void change_faculty_password(int client_socket, int faculty_id) {
    char buffer[1024] = {0};
    char new_password[50];
    
    send(client_socket, "Enter new password: ", 20, 0);
    read(client_socket, new_password, 50);
    
    sem_wait(&faculty_sem);
    for (int i = 0; i < faculty_count; i++) {
        if (faculty[i].id == faculty_id) {
            strcpy(faculty[i].password, new_password);
            send(client_socket, "Password changed successfully!\n", 30, 0);
            sem_post(&faculty_sem);
            save_data();
            return;
        }
    }
    sem_post(&faculty_sem);
    
    send(client_socket, "Faculty not found!\n", 19, 0);
}

void view_all_courses(int client_socket) {
    char buffer[1024] = {0};
    int found = 0;
    
    sem_wait(&course_sem);
    for (int i = 0; i < course_count; i++) {
        sprintf(buffer, "Course ID: %d\nName: %s\nFaculty: %s\nAvailable Seats: %d/%d\n\n",
               courses[i].id, courses[i].name, courses[i].faculty_name,
               courses[i].seats - courses[i].enrolled_count, courses[i].seats);
        send(client_socket, buffer, strlen(buffer), 0);
        found = 1;
        memset(buffer, 0, 1024);
    }
    sem_post(&course_sem);
    
    if (!found) {
        send(client_socket, "No courses available!\n", 22, 0);
    }
}

void enroll_course(int client_socket, int student_id) {
    char buffer[1024] = {0};
    int course_id;
    
    send(client_socket, "Enter course ID to enroll: ", 27, 0);
    read(client_socket, buffer, 1024);
    course_id = atoi(buffer);
    memset(buffer, 0, 1024);
    
    sem_wait(&course_sem);
    for (int i = 0; i < course_count; i++) {
        if (courses[i].id == course_id) {
            for (int j = 0; j < courses[i].enrolled_count; j++) {
                if (courses[i].enrolled_students[j] == student_id) {
                    send(client_socket, "You are already enrolled in this course!\n", 41, 0);
                    sem_post(&course_sem);
                    return;
                }
            }

            if (courses[i].enrolled_count >= courses[i].seats) {
                send(client_socket, "No seats available in this course!\n", 34, 0);
                sem_post(&course_sem);
                return;
            }

            courses[i].enrolled_students[courses[i].enrolled_count++] = student_id;
            send(client_socket, "Enrolled in course successfully!\n", 32, 0);
            sem_post(&course_sem);
            save_data();
            return;
        }
    }
    sem_post(&course_sem);
    
    send(client_socket, "Course not found!\n", 18, 0);
}

void drop_course(int client_socket, int student_id) {
    char buffer[1024] = {0};
    int course_id;
    
    send(client_socket, "Enter course ID to drop: ", 25, 0);
    read(client_socket, buffer, 1024);
    course_id = atoi(buffer);
    memset(buffer, 0, 1024);
    
    sem_wait(&course_sem);
    for (int i = 0; i < course_count; i++) {
        if (courses[i].id == course_id) {
            for (int j = 0; j < courses[i].enrolled_count; j++) {
                if (courses[i].enrolled_students[j] == student_id) {
                    for (int k = j; k < courses[i].enrolled_count - 1; k++) {
                        courses[i].enrolled_students[k] = courses[i].enrolled_students[k + 1];
                    }
                    courses[i].enrolled_count--;
                    send(client_socket, "Course dropped successfully!\n", 28, 0);
                    sem_post(&course_sem);
                    save_data();
                    return;
                }
            }
            
            send(client_socket, "You are not enrolled in this course!\n", 36, 0);
            sem_post(&course_sem);
            return;
        }
    }
    sem_post(&course_sem);
    
    send(client_socket, "Course not found!\n", 18, 0);
}

void view_enrolled_courses(int client_socket, int student_id) {
    char buffer[1024] = {0};
    int found = 0;
    
    sem_wait(&course_sem);
    for (int i = 0; i < course_count; i++) {
        for (int j = 0; j < courses[i].enrolled_count; j++) {
            if (courses[i].enrolled_students[j] == student_id) {
                sprintf(buffer, "Course ID: %d\nName: %s\nFaculty: %s\n\n",
                       courses[i].id, courses[i].name, courses[i].faculty_name);
                send(client_socket, buffer, strlen(buffer), 0);
                found = 1;
                memset(buffer, 0, 1024);
                break;
            }
        }
    }
    sem_post(&course_sem);
    
    if (!found) {
        send(client_socket, "You are not enrolled in any courses!\n", 36, 0);
    }
}

void change_student_password(int client_socket, int student_id) {
    char buffer[1024] = {0};
    char new_password[50];
    
    send(client_socket, "Enter new password: ", 20, 0);
    read(client_socket, new_password, 50);
    
    sem_wait(&student_sem);
    for (int i = 0; i < student_count; i++) {
        if (students[i].id == student_id) {
            strcpy(students[i].password, new_password);
            send(client_socket, "Password changed successfully!\n", 30, 0);
            sem_post(&student_sem);
            save_data();
            return;
        }
    }
    sem_post(&student_sem);
    
    send(client_socket, "Student not found!\n", 19, 0);
}

int main() {
    int server_fd, *new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    pthread_t thread_id;

    sem_init(&student_sem, 0, 1);
    sem_init(&faculty_sem, 0, 1);
    sem_init(&course_sem, 0, 1);

    load_data();

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server started on port %d\n", 8080);

    while (1) {
        new_socket = malloc(sizeof(int));
        if ((*new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            free(new_socket);
            continue;
        }

        printf("New connection from %s\n", inet_ntoa(address.sin_addr));

        if (pthread_create(&thread_id, NULL, handle_client, new_socket) < 0) {
            perror("could not create thread");
            free(new_socket);
            continue;
        }

        pthread_detach(thread_id);
    }

    sem_destroy(&student_sem);
    sem_destroy(&faculty_sem);
    sem_destroy(&course_sem);
    close(server_fd);
    save_data();

    return 0;
}

void load_data() {
    int fd = open("students.dat", O_RDONLY);
    if (fd != -1) {
        read(fd, &student_count, sizeof(int));
        read(fd, students, sizeof(Student) * student_count);
        close(fd);
    }

    fd = open("faculty.dat", O_RDONLY);
    if (fd != -1) {
        read(fd, &faculty_count, sizeof(int));
        read(fd, faculty, sizeof(Faculty) * faculty_count);
        close(fd);
    }

    fd = open("courses.dat", O_RDONLY);
    if (fd != -1) {
        read(fd, &course_count, sizeof(int));
        read(fd, courses, sizeof(Course) * course_count);
        close(fd);
    }
}

void save_data() {
    int fd = open("students.dat", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd != -1) {
        write(fd, &student_count, sizeof(int));
        write(fd, students, sizeof(Student) * student_count);
        close(fd);
    }

    fd = open("faculty.dat", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd != -1) {
        write(fd, &faculty_count, sizeof(int));
        write(fd, faculty, sizeof(Faculty) * faculty_count);
        close(fd);
    }

    fd = open("courses.dat", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd != -1) {
        write(fd, &course_count, sizeof(int));
        write(fd, courses, sizeof(Course) * course_count);
        close(fd);
    }
}

void *handle_client(void *arg) {
    int client_socket = *(int*)arg;
    free(arg); 

    char buffer[1024] = {0};
    int choice, id, authenticated = 0;
    char username[50], password[50];

    char *welcome = "......Welcome Back to Academia :: Course Registration......\n"
                    "Login Type\n"
                    "Enter Your Choice { 1.Admin , 2.Professor, 3.Student } : ";
    send(client_socket, welcome, strlen(welcome), 0);

    int bytes_read = read(client_socket, buffer, 1024-1);
    if (bytes_read <= 0) {
        close(client_socket);
        return NULL;
    }
    buffer[bytes_read] = '\0';
    choice = atoi(buffer);
    memset(buffer, 0, 1024);

    send(client_socket, "Enter username: ", 16, 0);
    bytes_read = read(client_socket, username, 49);
    if (bytes_read <= 0) {
        close(client_socket);
        return NULL;
    }
    username[bytes_read] = '\0';

    send(client_socket, "Enter password: ", 16, 0);
    bytes_read = read(client_socket, password, 49);
    if (bytes_read <= 0) {
        close(client_socket);
        return NULL;
    }
    password[bytes_read] = '\0';

    switch (choice) {
        case 1: 
            if (strcmp(username, "admin") == 0 && strcmp(password, "admin123") == 0) {
                authenticated = 1;
                admin_menu(client_socket);
            }
            break;
        case 2: 
            sem_wait(&faculty_sem);
            for (int i = 0; i < faculty_count; i++) {
                if (strcmp(username, faculty[i].name) == 0 && strcmp(password, faculty[i].password) == 0) {
                    authenticated = 1;
                    id = faculty[i].id;
                    sem_post(&faculty_sem);
                    faculty_menu(client_socket, id);
                    break;
                }
            }
            if (!authenticated) sem_post(&faculty_sem);
            break;
        case 3:
            sem_wait(&student_sem);
            for (int i = 0; i < student_count; i++) {
                if (strcmp(username, students[i].name) == 0 && 
                    strcmp(password, students[i].password) == 0 && 
                    students[i].is_active) {
                    authenticated = 1;
                    id = students[i].id;
                    sem_post(&student_sem);
                    student_menu(client_socket, id);
                    break;
                }
            }
            if (!authenticated) sem_post(&student_sem);
            break;
    }

    if (!authenticated) {
        send(client_socket, "EXIT: Authentication failed.\n", 29, 0);
        close(client_socket);
        return NULL;
    }

    close(client_socket);
    return NULL;
}

void admin_menu(int client_socket) {
    char buffer[1024] = {0};
    int choice;

    while (1) {
        char *menu = "\nWelcome to Admin Menu\n"
                     "1. Add Student\n"
                     "2. View Student Details\n"
                     "3. Add Faculty\n"
                     "4. View Faculty Details\n"
                     "5. Activate Student\n"
                     "6. Block Student\n"
                     "7. Modify Student Details\n"
                     "8. Modify Faculty Details\n"
                     "9. Logout and Exit\n"
                     "Enter Your Choice: \n"
                     "________________________________________________\n";
        send(client_socket, menu, strlen(menu), 0);

        read(client_socket, buffer, 1024);
        choice = atoi(buffer);
        memset(buffer, 0, 1024);

        switch (choice) {
            case 1: add_student(client_socket); break;
            case 2: view_student_details(client_socket); break;
            case 3: add_faculty(client_socket); break;
            case 4: view_faculty_details(client_socket); break;
            case 5: activate_student(client_socket); break;
            case 6: block_student(client_socket); break;
            case 7: modify_student_details(client_socket); break;
            case 8: modify_faculty_details(client_socket); break;
            case 9: return;
            default: send(client_socket, "Invalid choice!\n", 16, 0);
        }
    }
}

void faculty_menu(int client_socket, int faculty_id) {
    char buffer[1024] = {0};
    int choice;

    while (1) {
        char *menu = "\nWelcome to Faculty Menu\n"
                     "1. View Offering Courses\n"
                     "2. Add New Course\n"
                     "3. Remove Course from Catalog\n"
                     "4. Update Course Details\n"
                     "5. Change Password\n"
                     "6. Logout and Exit\n"
                     "Enter Your Choice: \n"
                     "________________________________________________\n";
        send(client_socket, menu, strlen(menu), 0);

        read(client_socket, buffer, 1024);
        choice = atoi(buffer);
        memset(buffer, 0, 1024);

        switch (choice) {
            case 1: view_offering_courses(client_socket, faculty_id); break;
            case 2: add_course(client_socket, faculty_id); break;
            case 3: remove_course(client_socket, faculty_id); break;
            case 4: update_course_details(client_socket, faculty_id); break;
            case 5: change_faculty_password(client_socket, faculty_id); break;
            case 6: return;
            default: send(client_socket, "Invalid choice!\n", 16, 0);
        }
    }
}

void student_menu(int client_socket, int student_id) {
    char buffer[1024] = {0};
    int choice;

    while (1) {
        char *menu = "\nWelcome to Student Menu\n"
                     "1. View All Courses\n"
                     "2. Enroll (pick) New Course\n"
                     "3. Drop Course\n"
                     "4. View Enrolled Course Details\n"
                     "5. Change Password\n"
                     "6. Logout and Exit\n"
                     "Enter Your Choice: \n"
                     "________________________________________________\n";
        send(client_socket, menu, strlen(menu), 0);

        read(client_socket, buffer, 1024);
        choice = atoi(buffer);
        memset(buffer, 0, 1024);

        switch (choice) {
            case 1: view_all_courses(client_socket); break;
            case 2: enroll_course(client_socket, student_id); break;
            case 3: drop_course(client_socket, student_id); break;
            case 4: view_enrolled_courses(client_socket, student_id); break;
            case 5: change_student_password(client_socket, student_id); break;
            case 6: return;
            default: send(client_socket, "Invalid choice!\n", 16, 0);
        }
    }
}
