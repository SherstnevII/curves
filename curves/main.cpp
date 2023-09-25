#define _USE_MATH_DEFINES

#include <cmath>
#include <iostream>
#include <vector>
#include <memory>
#include <random>
#include <algorithm>
#include <string>
#include <omp.h>


struct Point {
	double x{};
	double y{};
	double z{};
};

std::ostream& operator << (std::ostream& os, const Point& p)
{
	return os << "(" << p.x << ", " << p.y << ", " << p.z << ")";
}

class Curve {
	virtual std::string objectInfo() const = 0;
public:
	virtual ~Curve() {}

	// TODO: getType(), for better performance comparing to dynamic_cast

	virtual double radius() const = 0;

	virtual Point point(double t) const = 0;
	virtual Point derivative(double t) const = 0;

	friend std::ostream& operator << (std::ostream& os, const Curve& curve);
};

std::ostream& operator << (std::ostream& os, const Curve& curve)
{
	return os << curve.objectInfo();
}

class Circle : public Curve {
	double radius_;

	std::string objectInfo() const {
		return "Circle with r = " + std::to_string(radius_);
	}
public:
	Circle(double radius)
		: radius_(radius) {
	}

	double radius() const override {
		return radius_;
	}

	Point point(double t) const override {
		return { radius_ * cos(t), radius_ * sin(t), 0.0 };
	}

	Point derivative(double t) const override {
		return { -radius_ * sin(t), radius_ * cos(t), 0.0 };
	}
};

class Ellipse : public Curve {
	double radius_x_;
	double radius_y_;

	std::string objectInfo() const {
		return "Ellipse with rx = " + std::to_string(radius_x_) + ", ry = " + std::to_string(radius_y_);
	}
public:
	Ellipse(double radius_x, double radius_y) : radius_x_(radius_x), radius_y_(radius_y) {}

	double radius() const override {
		return std::max(radius_x_, radius_y_);
	}

	Point point(double t) const override {
		return { radius_x_ * cos(t), radius_y_ * sin(t), 0.0 };
	}

	Point derivative(double t) const override {
		return { -radius_x_ * sin(t), radius_y_ * cos(t), 0.0 };
	}
};

class Helix : public Curve {
	double radius_;
	double step_;

	std::string objectInfo() const {
		return "Helix with r = " + std::to_string(radius_) + ", s = " + std::to_string(step_);
	}
public:
	Helix(double radius, double step) : radius_(radius), step_(step) {}

	double radius() const override {
		return radius_;
	}

	Point point(double t) const override {
		return { radius_ * cos(t), radius_ * sin(t), step_ * t / (2 * M_PI) };
	}

	Point derivative(double t) const override {
		return { -radius_ * sin(t), radius_ * cos(t), step_ / (2 * M_PI) };
	}
};

int main() {
	const int size = 10;
	std::vector<std::unique_ptr<Curve>> curves;
	curves.reserve(size);

	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_int_distribution<int> type_dist(1, 3);
	std::uniform_real_distribution<double> radius_dist(0.1, 100.0);
	std::uniform_real_distribution<double> step_dist(0.1, 100.0);

	for (int i = 0; i < size; ++i) {
		switch (type_dist(mt)) {
		case 1:
			curves.push_back(std::make_unique<Circle>(radius_dist(mt)));
			break;
		case 2:
			curves.push_back(std::make_unique<Ellipse>(radius_dist(mt), radius_dist(mt)));
			break;
		default:
			curves.push_back(std::make_unique<Helix>(radius_dist(mt), step_dist(mt)));
			break;
		}
	}

	double t = M_PI / 4;
	for (auto& curve : curves) {
		std::cout << *curve << std::endl;
		std::cout << "Point at t = PI / 4: " << curve->point(t) << std::endl;
		std::cout << "Derivative at t = PI / 4: " << curve->derivative(t) << std::endl;
		std::cout << std::endl;
	}

	std::vector<Circle*> circles;
	for (const auto& curve : curves) {
		if (dynamic_cast<Circle*>(curve.get())) {
			// getType()
			circles.push_back(static_cast<Circle*>(curve.get()));
		}
	}

	std::sort(circles.begin(), circles.end(),
		[](const Circle* a, const Circle* b) {
			return a->radius() < b->radius();
		}
	);

	double radius_sum{};

	int i = 0;
	const int num_threads = 4;
	omp_set_num_threads(num_threads);
	#pragma omp parallel 
	{
		#pragma omp for
		for (i = 0; i < circles.size(); ++i) {
			radius_sum += circles[i]->radius();
		}
	}

	std::cout << "Total sum of radii of the circles: " << radius_sum << std::endl;

	return 0;
}