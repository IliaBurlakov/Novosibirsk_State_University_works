package calculator;

import calculator.Command;

import java.io.InputStream;
import java.util.HashMap;
import java.util.Map;
import java.util.Properties;

public class CommandFactory {
    private final Map<String, String> commandMap = new HashMap<>();

    public CommandFactory() {
        try (InputStream in = getClass().getResourceAsStream("commands.properties")) {
            if (in == null) {
                throw new RuntimeException("Can't find commands.properties file");
            }
            Properties props = new Properties();
            props.load(in);
            for (String cmd : props.stringPropertyNames()) {
                commandMap.put(cmd, props.getProperty(cmd));
            }
        } catch (Exception e) {
            throw new RuntimeException("An error occurred while loading properties of CommandFactory", e);
        }
    }

    public Command getCommand(String[] partsOfLine) {
        if (partsOfLine.length == 0) {
            return null;
        }

        String cmd = partsOfLine[0].toUpperCase();
        String className = commandMap.get(cmd);
        if (className == null) {
            System.err.println("Unknown command: " + cmd);
            return null;
        }

        try {
            Class<?> commandClass = Class.forName(className);
            return (Command) commandClass.getDeclaredConstructor().newInstance();
        } catch (Exception e) {
            // При ошибке рефлексии
            e.printStackTrace();
            return null;
        }
    }
}
