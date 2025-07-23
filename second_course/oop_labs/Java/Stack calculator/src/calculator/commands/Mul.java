package calculator.commands;

import calculator.Command;
import calculator.Context;
import calculator.exceptions.StackUnderflowException;

import java.util.Stack;

public class Mul implements Command {
    @Override
    public void execute(Context context, String[] args) throws StackUnderflowException {
        Stack<Double> stack = context.getStack();
        if (stack.size() < 2)
            throw new StackUnderflowException("MUL: not enough arguments.");
        double b = stack.pop();
        double a = stack.pop();
        stack.push(a * b);
    }
}