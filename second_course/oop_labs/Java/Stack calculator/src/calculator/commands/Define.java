package calculator.commands;

import calculator.Command;
import calculator.Context;
import calculator.exceptions.InvalidCommandArgumentException;

import java.util.Map;

public class Define implements Command {
    @Override
    public void execute(Context context, String[] args) throws InvalidCommandArgumentException {
        if (args.length != 3) {
            throw new InvalidCommandArgumentException("DEFINE: bad number of arguments. Format: DEFINE <VAR> <VALUE>");
        }
        String varName = args[1];
        String varValueStr = args[2];
        try {
            double varValue = Double.parseDouble(varValueStr);
            Map<String, Double> definitions = context.getDefinitions();
            definitions.put(varName, varValue);
        } catch (NumberFormatException e) {
            throw new InvalidCommandArgumentException("DEFINE: value of the variable must be a number.");
        }
    }
}
