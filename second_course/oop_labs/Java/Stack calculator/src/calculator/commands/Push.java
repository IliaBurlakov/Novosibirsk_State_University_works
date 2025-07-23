package calculator.commands;

import calculator.Command;
import calculator.Context;
import calculator.exceptions.InvalidCommandArgumentException;

import java.util.Map;

public class Push implements Command {
    @Override
    public void execute(Context context, String[] args) throws InvalidCommandArgumentException {
        if (args.length != 2)
            throw new InvalidCommandArgumentException("PUSH: bad number of arguments.");
        String value = args[1];
        try {
            double number = Double.parseDouble(value);
            context.getStack().push(number);
        } catch (NumberFormatException e) {
            Map<String, Double> definitions = context.getDefinitions();
            if (definitions.containsKey(value)) {
                context.getStack().push(definitions.get(value));
            } else {
                throw new InvalidCommandArgumentException("PUSH: unknown variable: " + value);
            }
        }
    }
}