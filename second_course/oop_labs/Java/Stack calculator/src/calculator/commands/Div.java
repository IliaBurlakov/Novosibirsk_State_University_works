package calculator.commands;

import calculator.Command;
import calculator.Context;
import calculator.exceptions.DivisionByZeroException;
import calculator.exceptions.StackUnderflowException;

import java.util.Stack;

public class Div implements Command {
    @Override
    public void execute(Context context, String[] args) throws StackUnderflowException, DivisionByZeroException {
        Stack<Double> stack = context.getStack();
        if (stack.size() < 2) {
            throw new StackUnderflowException("DIV: not enough arguments.");
        }
        double b = stack.pop();
        if (b == 0) {
            stack.push(b);
            throw new DivisionByZeroException("DIV: division by zero.");
        }
        double a = stack.pop();
        stack.push(a / b);
    }
}