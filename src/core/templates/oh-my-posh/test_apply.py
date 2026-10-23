"""Run with: python3 -B oh-my-posh/test_apply.py"""

import copy
from concurrent.futures import ThreadPoolExecutor
from contextlib import redirect_stdout
import io
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

import apply as theme


PALETTE = dict(zip(theme.ANSI, (
    "#202028", "#c85050", "#50a850", "#c8a850", "#5080c8", "#a850c8", "#50a8c8", "#c0c0c8",
    "#606068", "#f08080", "#80d880", "#f0d880", "#80b0f0", "#d880f0", "#80d8f0", "#ffffff",
)))
PALETTE.update(foreground="#e0e0e8", background="#101018")


def config(segment=None, **extra):
    return {"version": 4, "blocks": [{"type": "prompt", "alignment": "left", "segments": [
        segment or {"type": "text", "style": "plain", "template": "hello", "foreground": "#ff0000"}
    ]}], **extra}


class RecolorTests(unittest.TestCase):
    def test_arbitrary_palette_names_and_non_color_data(self):
        segment = {"type": "command", "style": "plain", "foreground": "p:brand_xyz", "background": "transparent",
                   "template": "{{ .Output }}", "options": {"command": "echo '#ff0000'", "label": "red"}}
        source = config(segment, palette={"brand_xyz": "#ff0000", "alias": "p:brand_xyz"}, var={"example": "#ff0000"})
        original = copy.deepcopy(source)
        result = theme.Recolor(PALETTE).convert(source)
        self.assertEqual(source, original)
        self.assertEqual(result["blocks"], source["blocks"])
        self.assertEqual(result["var"], source["var"])
        self.assertEqual(result["palette"], {"brand_xyz": PALETTE["red"], "alias": "p:brand_xyz"})

    def test_conditional_output_keeps_go_code_and_recolors_branches(self):
        expression = '{{ if eq .Env.TAG "#ff0000" }}#00ff00{{ else }}#ff0000{{ end }}'
        result = theme.Recolor(PALETTE).color(expression)
        self.assertEqual(result, '{{ if eq .Env.TAG "#ff0000" }}' + PALETTE["green"] + '{{ else }}' + PALETTE["red"] + '{{ end }}')
        computed = '{{ printf "#%06x" .Code }}'
        self.assertEqual(theme.Recolor(PALETTE).color(computed), computed)

    def test_shade_percentages_are_not_ansi_indices(self):
        value = "linear-gradient(#ff0000, darken(#00ff00, 20), p:existing)"
        result = theme.Recolor(PALETTE).color(value)
        self.assertEqual(result, f'linear-gradient({PALETTE["red"]}, darken({PALETTE["green"]}, 20), p:existing)')

    def test_go_delimiters_in_strings_and_comments_are_not_color_output(self):
        values = (
            '{{ printf "}}<#ff0000>" }}',
            r'{{ printf "escaped \"}}<#ff0000>" }}',
            r'{{ printf "backslash \\" }}',
            r"{{ '\'' }}",
            r"{{ '\\' }}",
            '{{ `}}<#ff0000>` }}',
            r'{{ `\` }}',
            '{{/* }}<#ff0000> */}}',
            '{{/* first */ "}}<#ff0000>" /* last */}}',
            '{{- /* multiline\n}}<#ff0000>\n*/ -}}',
        )
        for value in values:
            with self.subTest(value=value):
                mapper = theme.Recolor(PALETTE)
                self.assertEqual(mapper.markup(value), value)
                self.assertEqual(mapper.color(value), value)
                self.assertEqual(mapper.markup(f'<#00ff00>{value}<#ff0000>'),
                                 f'<{PALETTE["green"]}>{value}<{PALETTE["red"]}>')
                self.assertEqual(mapper.color(f'#00ff00{value}#ff0000'),
                                 f'{PALETTE["green"]}{value}{PALETTE["red"]}')

    def test_unterminated_go_actions_are_preserved(self):
        for tail in ('', ' .Value ', '"}}<#ff0000>', "'}}<#ff0000>", '`}}<#ff0000>',
                     '/* }}<#ff0000>', ' printf "trailing' + '\\'):
            with self.subTest(tail=tail):
                value = '{{' + tail
                mapper = theme.Recolor(PALETTE)
                self.assertEqual(mapper.markup(f'<#00ff00>{value}'), f'<{PALETTE["green"]}>{value}')
                self.assertEqual(mapper.color(f'#00ff00{value}'), f'{PALETTE["green"]}{value}')

    def test_markup_preserves_text_and_formatting_tags(self):
        source = '<b><#ff0000>error</></b> #00ff00 {{ .Value }} <https://example.org>'
        result = theme.Recolor(PALETTE).markup(source)
        self.assertEqual(result, f'<b><{PALETTE["red"]}>error</></b> #00ff00 {{{{ .Value }}}} <https://example.org>')
        original = '{{ printf "<#ff0000>%s</>" .Value }}'
        self.assertEqual(theme.Recolor(PALETTE).markup(original), original)

    def test_runtime_palette_selection_is_preserved(self):
        source = config(palettes={"template": '{{ if .Root }}one{{ else }}two{{ end }}', "list": {
            "one": {"status": "#ff0000"}, "two": {"status": "#00ff00"}}})
        result = theme.Recolor(PALETTE).convert(source)
        self.assertEqual(result["palettes"]["template"], source["palettes"]["template"])
        self.assertEqual(result["palettes"]["list"], {"one": {"status": PALETTE["red"]}, "two": {"status": PALETTE["green"]}})

    def test_names_indices_and_hex_override(self):
        mapper = theme.Recolor(PALETTE, {"#AbCdEf": "green"})
        self.assertEqual(mapper.color("red"), PALETTE["red"])
        self.assertEqual(mapper.color("1"), PALETTE["red"])
        self.assertEqual(mapper.color("9"), PALETTE["lightRed"])
        self.assertEqual(mapper.color("46"), PALETTE["green"])
        self.assertEqual(mapper.color("#abcdef"), PALETTE["green"])
        for special in ("p:my-color", "transparent", "parentBackground", "foreground", "299"):
            self.assertEqual(mapper.color(special), special)

    def test_neutral_text_contrast_is_corrected_on_static_background(self):
        source = config({"type": "text", "style": "powerline", "template": "hi", "foreground": "p:text", "background": "p:panel"},
                        palette={"text": "#ffffff", "panel": "#aaaaff"})
        mapper = theme.Recolor(PALETTE)
        result = mapper.convert(source)
        segment = result["blocks"][0]["segments"][0]
        foreground = theme.resolve_color(segment["foreground"], result["palette"])
        background = theme.resolve_color(segment["background"], result["palette"])
        self.assertGreaterEqual(theme.contrast(foreground, background), 4.5)
        self.assertEqual(result["palette"]["text"], PALETTE["foreground"])
        self.assertEqual(mapper.contrast_fixes, 1)

    def test_dynamic_background_keeps_original_foreground_relationship(self):
        source = config({"type": "text", "style": "plain", "template": "hi", "foreground": "p:text", "background": "p:panel",
                         "background_templates": ['{{ if .Root }}#000000{{ end }}']},
                        palette={"text": "#ffffff", "panel": "#aaaaff"})
        result = theme.Recolor(PALETTE).convert(source)
        self.assertEqual(result["blocks"][0]["segments"][0]["foreground"], "p:text")

    def test_inline_pair_can_gain_contrast_without_changing_go_code(self):
        result = theme.Recolor(PALETTE).markup('<#ffffff,#aaaaff>{{ .Path }}</>')
        pair = result.split(">", 1)[0][1:].split(",")
        self.assertGreaterEqual(theme.contrast(*pair), 4.5)
        self.assertTrue(result.endswith('>{{ .Path }}</>'))


class StorageTests(unittest.TestCase):
    def prompt_directory(self):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        directory = Path(temporary.name)
        theme.write_json(directory / "ptm-source.omp.json", config())
        theme.write_json(directory / "ptm-palette.json", PALETTE)
        return directory

    def test_pathological_go_actions_do_not_stall_apply(self):
        directory = self.prompt_directory()
        # Repeated openers also catch scanners that retry every unmatched '{{'.
        values = ['{{' + token * 100_000 + '<#ff0000>' for token in ('"', "'", '`', '/*', '{{')]
        source = config({"type": "text", "style": "plain", "templates": values, "foreground_templates": values})
        theme.write_json(directory / "ptm-source.omp.json", source)
        # Keep a regression from hanging the entire test runner.
        subprocess.run([sys.executable, "-B", theme.__file__, "--directory", str(directory), "apply"],
                       check=True, capture_output=True, text=True, timeout=5)
        self.assertEqual(theme.load_json(directory / "ptm.omp.json"), source)

    def test_apply_is_idempotent_and_keeps_source_and_prior_output_on_failure(self):
        with tempfile.TemporaryDirectory() as temp:
            directory = Path(temp)
            source = directory / "ptm-source.omp.json"
            colors = directory / "ptm-palette.json"
            output = directory / "ptm.omp.json"
            state = directory / "ptm-output-state.json"
            theme.write_json(source, config())
            theme.write_json(colors, PALETTE)
            original = source.read_bytes()
            with redirect_stdout(io.StringIO()):
                theme.apply(directory, False)
                before = output.stat().st_mtime_ns
                state_before = state.stat().st_mtime_ns
                contents = output.read_bytes()
                theme.apply(directory, False)
                self.assertEqual(output.stat().st_mtime_ns, before)
                self.assertEqual(state.stat().st_mtime_ns, state_before)
                theme.apply(directory, True)
                self.assertEqual(output.stat().st_mtime_ns, before)
            self.assertEqual(source.read_bytes(), original)
            colors.write_text('{}')
            with self.assertRaises(ValueError):
                theme.apply(directory, False)
            self.assertEqual(output.read_bytes(), contents)
            self.assertEqual(sorted(p.name for p in directory.iterdir()), ["ptm-output-state.json", "ptm-palette.json", "ptm-source.omp.json", "ptm.omp.json"])

    def test_unmanaged_output_is_preserved_even_when_identical(self):
        for value in (config(), theme.Recolor(PALETTE).convert(config())):
            with self.subTest(value=value), redirect_stdout(io.StringIO()):
                directory = self.prompt_directory()
                output = directory / "ptm.omp.json"
                theme.write_json(output, value)
                before = {p.name: p.read_bytes() for p in directory.iterdir()}
                theme.apply(directory, True)
                with self.assertRaisesRegex(ValueError, "unmanaged or was edited"):
                    theme.apply(directory, False)
                self.assertEqual({p.name: p.read_bytes() for p in directory.iterdir()}, before)

    def test_owned_output_changes_with_palette_but_protects_user_edits(self):
        directory = self.prompt_directory()
        output = directory / "ptm.omp.json"
        with redirect_stdout(io.StringIO()):
            theme.apply(directory, False)
            theme.write_json(directory / "ptm-palette.json", dict(PALETTE, red="#aa4455"))
            theme.apply(directory, False)
            self.assertEqual(theme.load_json(output)["blocks"][0]["segments"][0]["foreground"], "#aa4455")
            edited = theme.load_json(output)
            edited["blocks"][0]["segments"][0]["template"] = "my edit"
            theme.write_json(output, edited)
            before = {p.name: p.read_bytes() for p in directory.iterdir()}
            with self.assertRaisesRegex(ValueError, "unmanaged or was edited"):
                theme.apply(directory, False)
            self.assertEqual({p.name: p.read_bytes() for p in directory.iterdir()}, before)
            output.rename(directory / "saved.omp.json")
            theme.apply(directory, False)
            self.assertEqual(theme.load_json(directory / "saved.omp.json"), edited)
            self.assertEqual(theme.load_json(output)["blocks"][0]["segments"][0]["template"], "hello")

    def test_output_and_state_symlinks_are_never_followed(self):
        for name in ("ptm.omp.json", "ptm-output-state.json"):
            for exists in (True, False):
                with self.subTest(name=name, exists=exists):
                    directory = self.prompt_directory()
                    destination = directory / "unrelated.json"
                    if exists:
                        destination.write_text("user data")
                    link = directory / name
                    link.symlink_to(destination)
                    with self.assertRaisesRegex(ValueError, "regular file"):
                        theme.apply(directory, False)
                    self.assertTrue(link.is_symlink())
                    self.assertEqual(destination.read_text() if exists else destination.exists(), "user data" if exists else False)

    def test_unrecognized_state_is_preserved(self):
        for value in ([], {}, {"generator": "other-app", "sha256": ["a" * 64]}):
            with self.subTest(value=value):
                directory = self.prompt_directory()
                state = directory / "ptm-output-state.json"
                theme.write_json(state, value)
                before = state.read_bytes()
                with self.assertRaisesRegex(ValueError, "Unrecognized ownership"):
                    theme.apply(directory, False)
                self.assertEqual(state.read_bytes(), before)
                self.assertFalse((directory / "ptm.omp.json").exists())

    def test_interrupted_update_recovers_before_or_after_output_replacement(self):
        for fail_at in (2, 3):
            with self.subTest(fail_at=fail_at), redirect_stdout(io.StringIO()):
                directory = self.prompt_directory()
                theme.apply(directory, False)
                theme.write_json(directory / "ptm-palette.json", dict(PALETTE, red="#aa4455"))
                write = theme.write_json
                calls = 0

                def interrupted_write(*args, **kwargs):
                    nonlocal calls
                    calls += 1
                    if calls == fail_at:
                        raise OSError("interrupted")
                    return write(*args, **kwargs)

                with patch("apply.write_json", side_effect=interrupted_write):
                    with self.assertRaisesRegex(OSError, "interrupted"):
                        theme.apply(directory, False)
                theme.apply(directory, False)
                output = directory / "ptm.omp.json"
                self.assertEqual(theme.load_json(output)["blocks"][0]["segments"][0]["foreground"], "#aa4455")
                self.assertEqual(len(theme.load_json(directory / "ptm-output-state.json")["sha256"]), 1)

    def test_first_write_does_not_clobber_a_file_created_after_the_guard(self):
        directory = self.prompt_directory()
        output = directory / "ptm.omp.json"
        link = theme.os.link

        def create_collision(source, target):
            if target == output:
                target.write_text("arrived during apply")
            return link(source, target)

        with patch("apply.os.link", side_effect=create_collision):
            with self.assertRaises(FileExistsError):
                theme.apply(directory, False)
        self.assertEqual(output.read_text(), "arrived during apply")
        with self.assertRaisesRegex(ValueError, "unmanaged or was edited"):
            theme.apply(directory, False)

    def test_concurrent_applies_share_one_consistent_output(self):
        directory = self.prompt_directory()
        with redirect_stdout(io.StringIO()), ThreadPoolExecutor(max_workers=4) as workers:
            list(workers.map(lambda _: theme.apply(directory, False), range(8)))
        output = directory / "ptm.omp.json"
        self.assertEqual(theme.load_json(output), theme.Recolor(PALETTE).convert(config()))
        before = output.stat().st_mtime_ns
        with redirect_stdout(io.StringIO()):
            theme.apply(directory, False)
        self.assertEqual(output.stat().st_mtime_ns, before)

    def test_import_refuses_missing_config_and_its_own_output(self):
        with tempfile.TemporaryDirectory() as temp:
            directory = Path(temp)
            with self.assertRaises(ValueError):
                theme.import_config(directory, None, False)
            with self.assertRaises(ValueError):
                theme.import_config(directory, str(directory / 'missing.json'), False)
            target = directory / 'ptm.omp.json'
            theme.write_json(target, config())
            with self.assertRaises(ValueError):
                theme.import_config(directory, str(target), False)

    def test_import_uses_export_without_a_shell_and_does_not_edit_original(self):
        with tempfile.TemporaryDirectory() as temp:
            directory = Path(temp)
            original = directory / 'my theme.omp.toml'
            original.write_text('version = 4\n')
            with patch('apply.subprocess.run') as command, redirect_stdout(io.StringIO()):
                command.return_value.stdout = json.dumps(config())
                theme.import_config(directory, str(original), False)
                self.assertEqual(command.call_args.args[0], ["oh-my-posh", "config", "export", "--config", str(original), "--format", "json"])
                self.assertNotIn('shell', command.call_args.kwargs)
            self.assertEqual(original.read_text(), 'version = 4\n')
            with self.assertRaises(ValueError):
                theme.import_config(directory, str(original), False)

    def test_exporter_fallback_error_does_not_replace_imported_source(self):
        with tempfile.TemporaryDirectory() as temp:
            directory = Path(temp)
            original = directory / 'broken.omp.json'
            original.write_text('{ broken')
            imported = directory / 'ptm-source.omp.json'
            theme.write_json(imported, config())
            before = imported.read_bytes()
            with patch('apply.subprocess.run') as command:
                command.return_value.stdout = json.dumps(config({"type": "text", "template": " CONFIG PARSE ERROR "}))
                with self.assertRaises(ValueError):
                    theme.import_config(directory, str(original), True)
            self.assertEqual(imported.read_bytes(), before)


if __name__ == '__main__':
    unittest.main()
