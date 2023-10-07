#include <iostream>
#include <filesystem>
#include <fstream>
#include <utility>
#include <vector>
#include <ranges>

enum class BFInstruction
{
    INCR_PTR,      // >
    DECR_PTR,      // <
    INCR_BYTE,     // +
    DECR_BYTE,     // -
    OUT_BYTE,      // .
    READ_BYTE,     // ,
    WHILE_BEGIN,   // [
    WHILE_END,     // ]
    COMMENT,       // literally anything else
    END_OF_PROGRAM // end of file/program
};

class BFInterpreter
{
public:
    using Byte = unsigned char;
    static constexpr std::size_t BF_PTR_SIZE = 30'000;

private:
    BFInstruction m_inst;

    Byte m_tape[BFInterpreter::BF_PTR_SIZE];
    Byte *m_ptr;

    std::string_view m_input_file_path;

    std::istream &m_input_stream;
    std::ostream &m_output_stream;

    std::vector<BFInstruction> m_insts;
    std::vector<BFInstruction>::iterator m_insts_iter;

public:
    BFInterpreter(std::string_view input_file,
                  std::istream &inp_stream = std::cin,
                  std::ostream &out_stream = std::cout)
        : m_input_file_path{std::move(input_file)},
          m_input_stream{inp_stream},
          m_output_stream{out_stream}
    {
        if (!std::filesystem::exists(m_input_file_path))
        {
            std::cerr << "Invalid input file.\n";
            exit(1);
        }


        std::fill(std::begin(m_tape), std::end(m_tape), 0);
        m_ptr = m_tape;

        parse_insts();

        m_inst = BFInstruction::COMMENT;
    }

    [[nodiscard]] const std::string_view &get_path() const noexcept
    {
        return m_input_file_path;
    }

    void run()
    {
        while (true)
        {
            m_inst = next_inst();

            switch (m_inst)
            {
            case BFInstruction::INCR_PTR:
                ++m_ptr;
                break;

            case BFInstruction::DECR_PTR:
                --m_ptr;
                break;

            case BFInstruction::INCR_BYTE:
                ++*m_ptr;
                break;

            case BFInstruction::DECR_BYTE:
                --*m_ptr;
                break;

            case BFInstruction::OUT_BYTE:
                m_output_stream << *m_ptr;
                break;

            case BFInstruction::READ_BYTE:
                m_input_stream >> *m_ptr;
                break;

            case BFInstruction::WHILE_BEGIN:
                if (*m_ptr == 0)
                    skip_loop();
                break;

            case BFInstruction::WHILE_END:
                if (*m_ptr)
                    restart_loop();
                break;

            case BFInstruction::COMMENT:
                continue;

            case BFInstruction::END_OF_PROGRAM:
                return;

            default:
                std::unreachable();
            }
        }
    }

private:
    void parse_insts()
    {
        std::ifstream file;
        file.open(std::string{m_input_file_path}, std::ifstream::binary);
        file.seekg(0, std::ios::end);

        std::vector<char> bf_code;
        std::streampos file_length(file.tellg());
        if (!file_length)
            return;

        file.seekg(0, std::ios::beg);
        bf_code.resize(static_cast<std::size_t>(file_length));
        // +1 for end of program instruction
        m_insts.resize(static_cast<std::size_t>(file_length) + 1, BFInstruction::COMMENT);
        file.read(&bf_code.front(), static_cast<std::size_t>(file_length));

        BFInstruction next;
        for (const auto &[i, c] : std::views::enumerate(bf_code))
        {
            next = to_inst(c);
            if (next == BFInstruction::COMMENT)
                continue;

            m_insts[static_cast<std::size_t>(i)] = std::move(next);
        }

        m_insts.push_back(BFInstruction::END_OF_PROGRAM);
        m_insts_iter = m_insts.begin() - 1;
    }

    void skip_loop()
    {
        if (m_inst == BFInstruction::WHILE_BEGIN)
        {
            int loops = 1;
            while (loops > 0)
            {
                m_inst = next_inst();
                if (m_inst == BFInstruction::WHILE_BEGIN)
                    loops++;
                else if (m_inst == BFInstruction::WHILE_END)
                    loops--;
            }
        }
    }

    void restart_loop()
    {
        if (m_inst == BFInstruction::WHILE_END)
        {
            int loops = 1;
            while (loops > 0)
            {
                m_inst = prev_inst();
                if (m_inst == BFInstruction::WHILE_BEGIN)
                    loops--;
                else if (m_inst == BFInstruction::WHILE_END)
                    loops++;
            }
        }
    }

    [[nodiscard]] inline BFInstruction next_inst()
    {
        return m_insts_iter == m_insts.end() ? *m_insts_iter : *++m_insts_iter;
    }

    [[nodiscard]] inline BFInstruction prev_inst()
    {
        return m_insts_iter == m_insts.begin() ? *m_insts_iter : *--m_insts_iter;
    }

    [[nodiscard]] BFInstruction to_inst(char c) noexcept
    {
        switch (c)
        {
        case '>':
            return BFInstruction::INCR_PTR;

        case '<':
            return BFInstruction::DECR_PTR;

        case '+':
            return BFInstruction::INCR_BYTE;

        case '-':
            return BFInstruction::DECR_BYTE;

        case '.':
            return BFInstruction::OUT_BYTE;

        case ',':
            return BFInstruction::READ_BYTE;

        case '[':
            return BFInstruction::WHILE_BEGIN;

        case ']':
            return BFInstruction::WHILE_END;

        default:
            return BFInstruction::COMMENT;
        }

        std::unreachable();
    }
};

int main(int argc, char **argv)
{
    const char *USAGE = R"==(Usage

    bf-interpreter <path-to-source>
    )==";

    if (argc < 2)
    {
        std::cerr << USAGE;
        return 1;
    }

    BFInterpreter bf{argv[1]};
    bf.run();
}
