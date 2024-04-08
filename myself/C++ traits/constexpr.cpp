#include <iostream>

class Circle {
public:
    constexpr Circle(double r) : radius(r) {}
    constexpr double getRadius() const {
        return radius;
    }
    constexpr double getArea() const {
        return radius * radius * 3.14;
    }
private:
    const double radius;
};

int main() {
    constexpr Circle c(2.0);
    constexpr double area = c.getArea();
    std::cout << "Area of the circle with radius " << c.getRadius() << " is " << area << std::endl;
    return 0;
}