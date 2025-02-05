#include <iostream>
#include <iostream>
#include <cmath>                

using namespace std;

// Function to calculate the area of a circle
double calculateArea(double radius) {
    return M_PI * radius * radius;
}

// Function to calculate the perimeter of a circle
double calculatePerimeter(double radius) {
    return 2 * M_PI * radius;
}

// Function to check if the radius is valid
bool isValidRadius(double radius) {
    return radius > 0;
}

int main() {
    double radius;
    char choice;

    // Loop for multiple inputs
    do {
        cout << "Enter the radius of the circle: ";
        cin >> radius;

        if (isValidRadius(radius)) {
            double area = calculateArea(radius);
            double perimeter = calculatePerimeter(radius);

            cout << "Area: " << area << endl;
            cout << "Perimeter: " << perimeter << endl;
        } else {
            cout << "Invalid radius. Please enter a positive value." << endl;
        }

        // Ask if the user wants to calculate again
        cout << "Do you want to calculate again (y/n)? ";
        cin >> choice;

    } while (choice == 'y' || choice == 'Y');

    cout << "Thank you for using the circle calculator!" << endl;

    return 0;
}
