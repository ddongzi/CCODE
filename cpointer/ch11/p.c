// 操作码
enum OP{
    ADD,
    MUL,
    DIV,
    SUB
};
// 转换表
double add(double, double);
double mul(double, double);
double div(double, double);
double sub(double, double);

double (*oper_func[])(double, double) = {
    add,sub,mul,div
};

double calculate(double a, double b, enum OP op)
{
    return oper_func[op](a, b);
}
