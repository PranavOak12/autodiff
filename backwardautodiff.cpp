#include <iostream>
#include <cmath>
#include <vector>
#include <string>
#include <unordered_set>

#define VAR Var &

class Var;

void addoverload(Var *);
void subtractoverload(Var *);
void multiplyoverload(Var *);
void divideoverload(Var * );
void sinoverload(Var *);
void cosoverload(Var *);
void sqrtoverload(Var *);
void logoverload(Var *);
void negateoverload(Var *);
void nothing(Var *);

std::vector<Var *> globalvarptrs;

class Var
{
    private:
    static void calchelp(Var* node,std::unordered_set<Var*>& visited)
    {
        if(visited.find(node) != visited.end() || (node == nullptr))
        {
            return;
        }
        visited.insert(node);
        node->backop(node);
        calchelp(node->leftparent,visited);
        calchelp(node->rightparent,visited);
    }

    public:
    static int varcount;
    static std::string getdebugstring(int &num)
    {
        num++;
        int numcpy = num;
        std::string result;
        while (numcpy > 0)
        {
            numcpy--;
            char ch = 'a' + (numcpy % 26);
            result = ch + result;
            numcpy /= 26;
        }
        // std::cout << result << std::endl;
        return result;
    }

    static void calculategrad(Var* root)
    {
        root->grad = 1;
        std::unordered_set<Var*> visited;
        calchelp(root,visited);
    }
    double value;
    double grad;
    bool requiresgrad;
    Var *leftparent;
    Var *rightparent;
    void (*backop)(Var *);
    std::string dbgstr;

private:
    
    Var(double ivalue, double igrad = 0, bool irequiresgrad = true)
        : value(ivalue), grad(igrad), requiresgrad(irequiresgrad), leftparent(nullptr), rightparent(nullptr), backop(nothing)
    {
        globalvarptrs.push_back(this);
        dbgstr = getdebugstring(varcount);
    }

    Var(const Var &) = delete;
    Var &operator=(const Var &) = delete;

public:
    static Var &create(double ivalue, double igrad = 0, bool irequiresgrad = true)
    {
        Var *tmp = new Var(ivalue, igrad, irequiresgrad);
        return *tmp;
    }

    Var &operator+(const Var &other) const
    {
        Var &newvar = create(this->value + other.value);
        newvar.leftparent = const_cast<Var *>(this);
        newvar.rightparent = const_cast<Var *>(&other);
        newvar.backop = addoverload;
        return newvar;
    }

    Var &operator-(const Var &other) const
    {
        Var &newvar = create(this->value - other.value);
        newvar.leftparent = const_cast<Var *>(this);
        newvar.rightparent = const_cast<Var *>(&other);
        newvar.backop = subtractoverload;
        return newvar;
    }

    Var &operator*(const Var &other) const
    {
        Var &newvar = create(this->value * other.value);
        newvar.leftparent = const_cast<Var *>(this);
        newvar.rightparent = const_cast<Var *>(&other);
        newvar.backop = multiplyoverload;
        return newvar;
    }

    Var &operator/(const Var &other) const
    {
        Var &newvar = create(this->value / other.value);
        newvar.leftparent = const_cast<Var *>(this);
        newvar.rightparent = const_cast<Var *>(&other);
        newvar.backop = divideoverload;
        return newvar;
    }

    Var& operator-() const 
    {
        Var& newvar = create(-this->value);
        newvar.leftparent = const_cast<Var *>(this);
        newvar.backop = negateoverload;
        return newvar;
    }


    //var op constant
    Var& operator+(double constant) const
    {
        Var& constvar = Var::create(constant, 0.0, false);
        return *this + constvar;
    }

    Var& operator-(double constant) const 
    {
        Var& constvar = Var::create(constant, 0.0, false);
        return *this - constvar;
    }

    Var& operator*(double constant) const 
    {
        Var& constvar = Var::create(constant, 0.0, false);
        return *this * constvar;
    }

    Var& operator/(double constant) const 
    {
        Var& constvar = Var::create(constant, 0.0, false);
        return *this / constvar;
    }
};
int Var::varcount = 0;

/*
this will be called on var c back op
c = a + b
dz/da += dz/dc * dc/da
dz/da += grad * 1
*/

void addoverload(Var *var)
{
    if (var->leftparent->requiresgrad)
    {
        var->leftparent->grad += var->grad;
    }
    if (var->rightparent->requiresgrad)
    {
        var->rightparent->grad += var->grad;
    }
};

/*
this will be called on var c back op
c = a - b
dz/da += dz/dc * dc/da
dz/da += grad * 1
dz/db += dz/dc * dc/db
dz/db += grad * -1
*/
void subtractoverload(Var *var)
{
    if (var->leftparent->requiresgrad)
    {
        var->leftparent->grad += var->grad;
    }
    if (var->rightparent->requiresgrad)
    {
        var->rightparent->grad -= var->grad;
    }
};

/*
c = a*b
dz/da += dz/dc * dc*da
dz/da += grad * b
*/

void multiplyoverload(Var *var)
{
    if (var->leftparent->requiresgrad)
    {
        var->leftparent->grad += var->grad * var->rightparent->value;
    }
    if (var->rightparent->requiresgrad)
    {
        var->rightparent->grad += var->grad * var->leftparent->value;
    }
};

/*
c = a/b
dz/da += dz/dc * dc/da
dz/da += grad / b
dz/db -= grad * a / b^2
*/
void divideoverload(Var *var)
{
    if (var->leftparent->requiresgrad)
    {
        var->leftparent->grad += var->grad / var->rightparent->value;
    }
    if (var->rightparent->requiresgrad)
    {
        var->rightparent->grad -= var->grad * var->leftparent->value / (var->rightparent->value * var->rightparent->value);
    }
}

void negateoverload(Var* var) 
{
    if (var->leftparent->requiresgrad)
        var->leftparent->grad -= var->grad;
}


Var& sin(Var& var) 
{
    Var& newvar = Var::create(std::sin(var.value));
    newvar.leftparent = &var;
    newvar.backop = sinoverload;
    return newvar;
}

Var& cos(Var& var) 
{
    Var& newvar = Var::create(std::cos(var.value));
    newvar.leftparent = &var;
    newvar.backop = cosoverload;
    return newvar;
}

Var& sqrt(Var& var) 
{
    Var& newvar = Var::create(std::sqrt(var.value));
    newvar.leftparent = &var;
    newvar.backop = sqrtoverload;
    return newvar;
}

Var& exp(Var& var) 
{
    Var& newvar = Var::create(std::exp(var.value));
    newvar.leftparent = &var;
    newvar.backop = [](Var* v) {
        if (v->leftparent->requiresgrad)
            v->leftparent->grad += v->grad * std::exp(v->leftparent->value);
    };
    return newvar;
}

Var& log(Var& var) 
{
    Var& newvar = Var::create(std::log(var.value));
    newvar.leftparent = &var;
    newvar.backop = logoverload;
    return newvar;
}


void sinoverload(Var *var)
{
    if (var->leftparent->requiresgrad)
    {
        var->leftparent->grad += var->grad * std::cos(var->leftparent->value);
    }
}

void cosoverload(Var *var)
{
    if (var->leftparent->requiresgrad)
    {
        var->leftparent->grad -= var->grad * std::sin(var->leftparent->value);
    }
}

void sqrtoverload(Var *var)
{
    if (var->leftparent->requiresgrad)
    {
        var->leftparent->grad += var->grad * 0.5 / std::sqrt(var->leftparent->value);
    }
}

void logoverload(Var *var)
{
    if (var->leftparent->requiresgrad)
    {
        var->leftparent->grad += var->grad / var->leftparent->value;
    }
}

void nothing(Var *) {

};


//constant op Var
Var& operator+(double constant, Var& var) {
    Var& constvar = Var::create(constant, 0.0, false);
    return constvar + var;
}

Var& operator-(double constant, Var& var) {
    Var& constvar = Var::create(constant, 0.0, false);
    return constvar - var;
}

Var& operator*(double constant, Var& var) {
    Var& constvar = Var::create(constant, 0.0, false);
    return constvar * var;
}

Var& operator/(double constant, Var& var) {
    Var& constvar = Var::create(constant, 0.0, false);
    return constvar / var;
}


int main()
{
    VAR a = Var::create(2.0);
    VAR b = Var::create(3.0);
    VAR c = Var::create(1.0);
    VAR f = sin(a * b + 2.0) + sqrt(c / 4.0);
    Var::calculategrad(&f);
    std::cout << f.grad << " " << a.grad << " " << b.grad << " " <<c.grad << std::endl;
}


//not working due to pointer issues
// #include <iostream>
// #include <cmath>
// #include <vector>
// #include <unordered_set>

// class Var;

// void addoverload(Var*);
// void subtractoverload(Var*);
// void multiplyoverload(Var*);
// void divideoverload(Var*);
// void sinoverload(Var*);
// void cosoverload(Var*);
// void sqrtoverload(Var*);
// void logoverload(Var*);
// void negateoverload(Var*);

// class Var
// {

//     private:
//     static void calchelp(Var* node,std::unordered_set<Var*>& visited)
//     {
//         if (visited.count(node) || !node->backop) return;
//         visited.insert(node);
//         node->backop(node);
//         for(auto parent : node->parents)
//         {
//             calchelp(parent,visited);
//         }
//     }
//     public:
//     static void calulategrad(Var* root)
//     {
//         root->grad = 1;
//         std::unordered_set<Var*> visited;
//         calchelp(root,visited);
//     }
//     double value;
//     double grad;
//     bool requiresgrad;
//     std::vector<Var*> parents;
//     // convention in parents vector we always have left parent as 1st index and right parent as 2nd and in case of unary functions like sqrt sin log etc second parent is null

//     void (*backop) (Var*) = nullptr;
//     Var(double ivalue,double igrad = 0,double irequiresgrad = true)
//     : value(ivalue) , grad(igrad) , requiresgrad(irequiresgrad)
//     {}

//     //Var op Var
//     Var operator+ (const Var& other) const
//     {
//         Var newvar = Var(this->value + other.value,0,true);
//         newvar.parents.push_back(const_cast<Var*>(this));
//         newvar.parents.push_back(const_cast<Var*>(&other));
//         newvar.backop = addoverload;
//         return newvar;
//     }

//     Var operator- (const Var& other) const
//     {
//         Var newvar = Var(this->value - other.value,0,true);
//         newvar.parents.push_back(const_cast<Var*>(this));
//         newvar.parents.push_back(const_cast<Var*>(&other));
//         newvar.backop = subtractoverload;
//         return newvar;

//     }

//     Var operator* (const Var& other) const
//     {
//         Var newvar = Var(this->value * other.value,0,true);
//         newvar.parents.push_back(const_cast<Var*>(this));
//         newvar.parents.push_back(const_cast<Var*>(&other));
//         newvar.backop = multiplyoverload;
//         return newvar;
//     }

//     Var operator/ (const Var& other) const
//     {
//         Var newvar = Var(this->value / other.value,0,true);
//         newvar.parents.push_back(const_cast<Var*>(this));
//         newvar.parents.push_back(const_cast<Var*>(&other));
//         newvar.backop = divideoverload;
//         return newvar;
//     }

//     // Binary operators with double (mark double as constant)
//     Var operator+(double other) const
//     {
//         Var constvar(other, 0.0, false);
//         return *this + constvar;
//     }

//     Var operator-(double other) const
//     {
//         Var constvar(other, 0.0, false);
//         return *this - constvar;
//     }

//     Var operator*(double other) const
//     {
//         Var constvar(other, 0.0, false);
//         return *this * constvar;
//     }

//     Var operator/(double other) const
//     {
//         Var constvar(other, 0.0, false);
//         return *this / constvar;
//     }

//         Var sin() {
//         Var newvar(std::sin(this->value));
//         newvar.parents = {this, nullptr};
//         newvar.backop = sinoverload;
//         return newvar;
//     }

//     Var cos() {
//         Var newvar(std::cos(this->value));
//         newvar.parents = {this, nullptr};
//         newvar.backop = cosoverload;
//         return newvar;
//     }

//     Var sqrt() {
//         Var newvar(std::sqrt(this->value));
//         newvar.parents = {this, nullptr};
//         newvar.backop = sqrtoverload;
//         return newvar;
//     }

//     Var log() {
//         Var newvar(std::log(this->value));
//         newvar.parents = {this, nullptr};
//         newvar.backop = logoverload;
//         return newvar;
//     }

//     Var operator-() {
//         Var newvar(-this->value);
//         newvar.parents = {this, nullptr};
//         newvar.backop = negateoverload;
//         return newvar;
//     }
// };

// // c = a * b
// // dz/da += dz/dc * dc/da
// // dz/da += dz/dc * b
// void multiplyoverload(Var* var) {
//     if (var->parents[0]->requiresgrad)
//         var->parents[0]->grad += var->grad * var->parents[1]->value;
//     if (var->parents[1]->requiresgrad)
//         var->parents[1]->grad += var->grad * var->parents[0]->value;
// }

// // c = a + b
// // dz/da += dz/dc * dc/da
// // dz/da += grad * 1
// void addoverload(Var* var)
// {
//     if (var->parents[0]->requiresgrad)
//         var->parents[0]->grad += var->grad;
//     if (var->parents[1]->requiresgrad)
//         var->parents[1]->grad += var->grad;
// }

// // c = a - b
// // dz/da += dz/dc * dc/da
// // dz/da += grad * 1
// // dz/db += dz/dc * dc/db
// // dz/db += grad * -1
// void subtractoverload(Var* var)
// {
//     if (var->parents[0]->requiresgrad)
//         var->parents[0]->grad += var->grad;
//     if (var->parents[1]->requiresgrad)
//         var->parents[1]->grad -= var->grad;
// }

// // c = a/b
// // dz/da += dz/dc * dc/da
// // dz/da += grad * 1/b
// // dz/db += dz/dc * dc/db
// // dz/db += grad * -a/b^2
// void divideoverload(Var* var) {
//     if (var->parents[0]->requiresgrad)
//         var->parents[0]->grad += var->grad / var->parents[1]->value;
//     if (var->parents[1]->requiresgrad)
//         var->parents[1]->grad -= var->grad * var->parents[0]->value / std::pow(var->parents[1]->value, 2);
// }

// Var operator+(double c, const Var& var)
// {
//     Var constvar(c, 0.0, false);
//     return constvar + var;
// }

// Var operator-(double c, const Var& var)
// {
//     Var constvar(c, 0.0, false);
//     return constvar - var;
// }

// Var operator*(double c, const Var& var)
// {
//     Var constvar(c, 0.0, false);
//     return constvar * var;
// }

// Var operator/(double c, const Var& var)
// {
//     Var constvar(c, 0.0, false);
//     return constvar / var;
// }

// // Unary backward operations
// void sinoverload(Var* var)
// {
//     if (var->parents[0]->requiresgrad)
//         var->parents[0]->grad += var->grad * std::cos(var->parents[0]->value);
// }

// void cosoverload(Var* var)
// {
//     if (var->parents[0]->requiresgrad)
//         var->parents[0]->grad -= var->grad * std::sin(var->parents[0]->value);
// }

// void sqrtoverload(Var* var)
// {
//     if (var->parents[0]->requiresgrad)
//         var->parents[0]->grad += var->grad * 0.5 / std::sqrt(var->parents[0]->value);
// }

// void logoverload(Var* var)
// {
//     if (var->parents[0]->requiresgrad)
//         var->parents[0]->grad += var->grad / var->parents[0]->value;
// }

// void negateoverload(Var* var)
// {
//     if (var->parents[0]->requiresgrad)
//         var->parents[0]->grad -= var->grad;
// }

// int main()
// {
//     Var a(5.0,0,true);
//     Var b(6.0,0,true);
//     Var c = a * b + a + b + 1/(a*b*b).sqrt();
//     Var::calulategrad(&c);
//     std::cout << a.grad << " " << b.grad << std::endl;
// }