package calculator.commands;

import calculator.Command;
import calculator.Context;
import calculator.exceptions.StackUnderflowException;

public class Pop implements Command {
    @Override
    public void execute(Context context, String[] args) throws StackUnderflowException {
        if (context.getStack().isEmpty())
            throw new StackUnderflowException("POP: stack is empty.");
        context.getStack().pop();
    }
}