package calculator.commands;

import calculator.Command;
import calculator.Context;
import calculator.exceptions.StackUnderflowException;

import java.util.Stack;

public class Print implements Command {
    @Override
    public void execute(Context context, String[] args) throws StackUnderflowException {
        Stack<Double> stack = context.getStack();
        if (stack.isEmpty()) {
            throw new StackUnderflowException("PRINT: Stack is empty.");
        }
        double value = stack.peek();
        System.out.println(value);
    }
}
