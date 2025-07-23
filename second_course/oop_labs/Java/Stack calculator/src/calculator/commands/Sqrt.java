package calculator.commands;

import calculator.Command;
import calculator.Context;
import calculator.exceptions.RootOfANegativeNumberException;
import calculator.exceptions.StackUnderflowException;

import java.util.Stack;

public class Sqrt implements Command {
    @Override
    public void execute(Context context, String[] args) throws StackUnderflowException, RootOfANegativeNumberException {
        Stack<Double> stack = context.getStack();
        if (stack.isEmpty())
            throw new StackUnderflowException("SQRT: Stack is empty.");
        double value = stack.pop();
        if (value < 0)
            throw new RootOfANegativeNumberException("SQRT: it's impossible to apply sqrt to a negative number.");
        stack.push(Math.sqrt(value));
    }
}
