#include <cmath>
#include <iostream>

class Var
{
    public:
    double value;
    double grad;
    

    Var(double ivalue,double igrad = 0)
    : value(ivalue) , grad(igrad)
    { }

    //Var op Var

    Var operator+(const Var other) const
    {
        return Var(other.value+this->value,other.grad+this->grad);
    }

    Var operator*(const Var other) const
    {
        return Var(other.value * this->value,other.value * this->grad + this->value * other.grad);
    }

    Var operator-(const Var other) const
    {
        return Var(this->value - other.value,this->grad - other.grad);
    }

    Var operator/(const Var other) const
    {
        return Var(this->value / other.value,(this->grad * other.value - other.grad * this->value)/(other.value * other.value));
    }

    // Var op double

    Var operator+(double c) const 
    {
        return Var(value + c, grad);
    }

    Var operator-(double c) const 
    {
        return Var(value - c, grad);
    }

    Var operator* (double c) const
    {
        return Var(value * c,grad * c);
    }

    Var operator/ (double c) const
    {
        return Var(value/c,grad/c);
    }
    
    // Unary minuss
    Var operator-() const 
    {
        return {-value, -grad};
    }
};

// double op varr

Var operator*(double c,const Var& var)
{
    return Var(var.value * c, var.grad * c);
}

Var operator/(double c,const Var& var) 
{
    return Var(c / var.value , (-1/(var.value*var.value)) * var.grad);
}

Var operator-(double c,const Var& var)
{
    return Var(c - var.value,-var.grad);
}

Var operator+(double c,const Var& var)
{
    return Var(c + var.value,var.grad);
}

Var sin(const Var ivar)
{
    return Var(std::sin(ivar.value),std::cos(ivar.value)* ivar.grad);
}


Var cos(const Var& ivar)
{
    return Var(std::cos(ivar.value),-std::sin(ivar.value) * ivar.grad);
}

Var tan(const Var& ivar) {
    double val = std::tan(ivar.value);
    double grad = (1.0 / (std::cos(ivar.value) * std::cos(ivar.value))) * ivar.grad;
    return Var(val, grad);
}


Var exp(const Var& ivar) {
    double val = std::exp(ivar.value);
    double grad = val * ivar.grad;
    return Var(val, grad);
}

Var ln(const Var& ivar) {
    double val = std::log(ivar.value);
    double der = (1.0 / ivar.value) * ivar.grad;
    return Var(val, der);
}


Var sqrt(const Var& v) {
    double val  = std::sqrt(v.value);
    double grad = (1.0 / (2.0 * val)) * v.grad;
    return Var(val, grad);
}


int main() 
{
    
    Var a = Var(5.0,0.0);
    Var b = Var(6.0,1.0);
    Var c = a * b*b + a + b ;   
    std::cout << c.grad << std::endl;
    return 0;
}

