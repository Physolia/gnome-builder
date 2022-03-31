/* ide-preferences-builtin.c
 *
 * Copyright 2015-2019 Christian Hergert <chergert@redhat.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#define G_LOG_DOMAIN "ide-preferences-builtin"

#include "config.h"

#include <glib/gi18n.h>

#include <adwaita.h>
#include <gtksourceview/gtksource.h>
#include <libpeas/peas.h>

#include "ide-preferences-builtin-private.h"

#if 0
static gint
sort_plugin_info (gconstpointer a,
                  gconstpointer b)
{
  PeasPluginInfo *plugin_info_a = (PeasPluginInfo *)a;
  PeasPluginInfo *plugin_info_b = (PeasPluginInfo *)b;
  const gchar *name_a = peas_plugin_info_get_name (plugin_info_a);
  const gchar *name_b = peas_plugin_info_get_name (plugin_info_b);

  if (name_a == NULL || name_b == NULL)
    return g_strcmp0 (name_a, name_b);

  return g_utf8_collate (name_a, name_b);
}

static void
ide_preferences_builtin_register_plugins (DzlPreferences *preferences)
{
  PeasEngine *engine;
  const GList *list;
  GList *copy;
  guint i = 0;

  g_assert (DZL_IS_PREFERENCES (preferences));

  engine = peas_engine_get_default ();
  list = peas_engine_get_plugin_list (engine);

  dzl_preferences_add_page (preferences, "plugins", _("Extensions"), 700);
  dzl_preferences_add_list_group (preferences, "plugins", "plugins", _("Extensions"), GTK_SELECTION_NONE, 100);

  copy = g_list_sort (g_list_copy ((GList *)list), sort_plugin_info);

  for (const GList *iter = copy; iter; iter = iter->next, i++)
    {
      PeasPluginInfo *plugin_info = iter->data;
      g_autofree gchar *path = NULL;
      g_autofree gchar *keywords = NULL;
      const gchar *desc;
      const gchar *name;

      if (peas_plugin_info_is_hidden (plugin_info))
        continue;

      name = peas_plugin_info_get_name (plugin_info);
      desc = peas_plugin_info_get_description (plugin_info);
      keywords = g_strdup_printf ("%s %s", name, desc);
      path = g_strdup_printf ("/org/gnome/builder/plugins/%s/",
                              peas_plugin_info_get_module_name (plugin_info));

      dzl_preferences_add_switch (preferences, "plugins", "plugins", "org.gnome.builder.plugin", "enabled", path, NULL, name, desc, keywords, i);
    }

  g_list_free (copy);
}

static void
ide_preferences_builtin_register_appearance (DzlPreferences *preferences)
{
  GtkSourceStyleSchemeManager *manager;
  const gchar * const *scheme_ids;
  GtkWidget *bin;
  gint i;
  gint follow_style;

  dzl_preferences_add_page (preferences, "appearance", _("Appearance"), 0);

  dzl_preferences_add_list_group (preferences, "appearance", "style", _("Style"), GTK_SELECTION_NONE, 0);
  follow_style = dzl_preferences_add_radio (preferences, "appearance", "style", "org.gnome.builder", "style-variant", NULL, "\"follow\"", _("System"), NULL, _("follow"), 0);
  dzl_preferences_add_radio (preferences, "appearance", "style", "org.gnome.builder", "style-variant", NULL, "\"light\"", _("Light"), NULL, NULL, 0);
  dzl_preferences_add_radio (preferences, "appearance", "style", "org.gnome.builder", "style-variant", NULL, "\"dark\"", _("Dark"), NULL, NULL, 0);

  dzl_preferences_add_list_group (preferences, "appearance", "editor", _("Editor"), GTK_SELECTION_NONE, 0);
  dzl_preferences_add_switch (preferences, "appearance", "editor", "org.gnome.builder.editor", "show-grid-lines", NULL, NULL, _("Grid Pattern"), _("Display a grid pattern underneath source code"), NULL, 0);

  dzl_preferences_add_list_group (preferences, "appearance", "font", _("Font"), GTK_SELECTION_NONE, 10);
  dzl_preferences_add_font_button (preferences, "appearance", "font", "org.gnome.builder.editor", "font-name", _("Editor"), C_("Keywords", "editor font monospace"), 0);
  dzl_preferences_add_spin_button (preferences, "appearance", "font", "org.gnome.builder.editor", "line-spacing", NULL, _("Line Spacing"), _("Number of pixels above and below editor lines"), C_("Keywords", "editor line spacing font monospace"), 0);

  manager = gtk_source_style_scheme_manager_get_default ();
  scheme_ids = gtk_source_style_scheme_manager_get_scheme_ids (manager);

  dzl_preferences_add_list_group (preferences, "appearance", "schemes", _("Color Scheme"), GTK_SELECTION_NONE, 20);

  for (i = 0; scheme_ids [i]; i++)
    {
      g_autofree gchar *variant_str = NULL;
      GtkSourceStyleScheme *scheme;
      const gchar *title;

      variant_str = g_strdup_printf ("\"%s\"", scheme_ids [i]);
      scheme = gtk_source_style_scheme_manager_get_scheme (manager, scheme_ids [i]);
      title = gtk_source_style_scheme_get_name (scheme);

      dzl_preferences_add_radio (preferences, "appearance", "schemes", "org.gnome.builder.editor", "style-scheme-name", NULL, variant_str, title, NULL, title, i);
    }

  if (!adw_style_manager_get_system_supports_color_schemes (adw_style_manager_get_default ()))
    {
      bin = dzl_preferences_get_widget (preferences, follow_style);
      gtk_widget_set_sensitive (bin, FALSE);
    }
}

static void
ide_preferences_builtin_register_keyboard (DzlPreferences *preferences)
{
  dzl_preferences_add_page (preferences, "keyboard", _("Keyboard"), 400);

  dzl_preferences_add_list_group (preferences, "keyboard", "mode", _("Emulation"), GTK_SELECTION_NONE, 0);
  dzl_preferences_add_radio (preferences, "keyboard", "mode", "org.gnome.builder.editor", "keybindings", NULL, "\"default\"", _("Builder"), _("Default keybinding mode which mimics gedit"), NULL, 0);

  dzl_preferences_add_list_group (preferences, "keyboard", "movements", _("Movement"), GTK_SELECTION_NONE, 100);
  dzl_preferences_add_switch (preferences, "keyboard", "movements", "org.gnome.builder.editor", "smart-home-end", NULL, NULL, _("Smart Home and End"), _("Home moves to first non-whitespace character"), NULL, 0);
  dzl_preferences_add_switch (preferences, "keyboard", "movements", "org.gnome.builder.editor", "smart-backspace", NULL, NULL, _("Smart Backspace"), _("Backspace will remove extra space to keep you aligned with your indentation"), NULL, 100);
}

static void
ide_preferences_builtin_register_editor (DzlPreferences *preferences)
{
  dzl_preferences_add_page (preferences, "editor", _("Editor"), 100);

  dzl_preferences_add_list_group (preferences, "editor", "general", _("General"), GTK_SELECTION_NONE, -5);
  dzl_preferences_add_switch (preferences, "editor", "general", "org.gnome.builder", "show-open-files", NULL, NULL, _("Display list of open files"), _("Display the list of all open files in the project sidebar"), NULL, 0);
  dzl_preferences_add_switch (preferences, "editor", "general", "org.gnome.builder", "format-on-save", NULL, NULL, _("Reformat code on save"), _("Reformat current file on save"), NULL, 5);

  dzl_preferences_add_list_group (preferences, "editor", "position", _("Cursor"), GTK_SELECTION_NONE, 0);
  dzl_preferences_add_switch (preferences, "editor", "position", "org.gnome.builder.editor", "restore-insert-mark", NULL, NULL, _("Restore cursor position"), _("Restore cursor position when a file is reopened"), NULL, 0);
  dzl_preferences_add_spin_button (preferences, "editor", "position", "org.gnome.builder.editor", "scroll-offset", NULL, _("Scroll Offset"), _("Minimum number of lines to keep above and below the cursor"), NULL, 10);
  dzl_preferences_add_spin_button (preferences, "editor", "position", "org.gnome.builder.editor", "overscroll", NULL, _("Overscroll"), _("Allow the editor to scroll past the end of the buffer"), NULL, 20);

  dzl_preferences_add_list_group (preferences, "editor", "text-wrapping", _("Text Wrapping"), GTK_SELECTION_NONE, 0);
  dzl_preferences_add_radio (preferences, "editor", "text-wrapping", "org.gnome.builder.editor", "wrap-text", NULL, "\"never\"", _("Never"), NULL, NULL, 0);
  dzl_preferences_add_radio (preferences, "editor", "text-wrapping", "org.gnome.builder.editor", "wrap-text", NULL, "\"whitespace\"", _("In Between Words"), NULL, NULL, 0);
  dzl_preferences_add_radio (preferences, "editor", "text-wrapping", "org.gnome.builder.editor", "wrap-text", NULL, "\"always\"", _("Always"), NULL, NULL, 0);

  dzl_preferences_add_list_group (preferences, "editor", "line", _("Line Information"), GTK_SELECTION_NONE, 50);
  dzl_preferences_add_switch (preferences, "editor", "line", "org.gnome.builder.editor", "show-line-numbers", NULL, NULL, _("Line numbers"), _("Show line number at beginning of each line"), NULL, 0);
  dzl_preferences_add_switch (preferences, "editor", "line", "org.gnome.builder.editor", "show-relative-line-numbers", NULL, NULL, _("Relative line numbers"), _("Show line numbers relative to the cursor line"), NULL, 0);
  dzl_preferences_add_switch (preferences, "editor", "line", "org.gnome.builder.editor", "show-line-changes", NULL, NULL, _("Line changes"), _("Show if a line was added or modified next to line number"), NULL, 1);
  dzl_preferences_add_switch (preferences, "editor", "line", "org.gnome.builder.editor", "show-line-diagnostics", NULL, NULL, _("Line diagnostics"), _("Show an icon next to line numbers indicating type of diagnostic"), NULL, 2);

  dzl_preferences_add_list_group (preferences, "editor", "highlight", _("Highlight"), GTK_SELECTION_NONE, 100);
  dzl_preferences_add_switch (preferences, "editor", "highlight", "org.gnome.builder.editor", "highlight-current-line", NULL, NULL, _("Current line"), _("Make current line stand out with highlights"), NULL, 0);
  dzl_preferences_add_switch (preferences, "editor", "highlight", "org.gnome.builder.editor", "highlight-matching-brackets", NULL, NULL, _("Matching brackets"), _("Highlight matching brackets based on cursor position"), NULL, 1);

  dzl_preferences_add_list_group (preferences, "editor", "overview", _("Code Overview"), GTK_SELECTION_NONE, 100);
  dzl_preferences_add_switch (preferences, "editor", "overview", "org.gnome.builder.editor", "show-map", NULL, NULL, _("Show overview map"), _("A zoomed out view to enhance navigating source code"), NULL, 0);
  dzl_preferences_add_switch (preferences, "editor", "overview", "org.gnome.builder.editor", "auto-hide-map", NULL, NULL, _("Automatically hide overview map"), _("Automatically hide map when editor loses focus"), NULL, 1);

  dzl_preferences_add_list_group (preferences, "editor", "draw-spaces", _("Visible Whitespace Characters"), GTK_SELECTION_NONE, 400);
  dzl_preferences_add_radio (preferences, "editor", "draw-spaces", "org.gnome.builder.editor", "draw-spaces", NULL, "\"space\"", _("Spaces"), NULL, NULL, 0);
  dzl_preferences_add_radio (preferences, "editor", "draw-spaces", "org.gnome.builder.editor", "draw-spaces", NULL, "\"tab\"", _("Tabs"), NULL, NULL, 1);
  dzl_preferences_add_radio (preferences, "editor", "draw-spaces", "org.gnome.builder.editor", "draw-spaces", NULL, "\"newline\"", _("New line and carriage return"), NULL, NULL, 2);
  dzl_preferences_add_radio (preferences, "editor", "draw-spaces", "org.gnome.builder.editor", "draw-spaces", NULL, "\"nbsp\"", _("Non-breaking spaces"), NULL, NULL, 3);
  dzl_preferences_add_radio (preferences, "editor", "draw-spaces", "org.gnome.builder.editor", "draw-spaces", NULL, "\"text\"", _("Spaces inside of text"), NULL, NULL, 4);
  dzl_preferences_add_radio (preferences, "editor", "draw-spaces", "org.gnome.builder.editor", "draw-spaces", NULL, "\"trailing\"", _("Trailing Only"), NULL, NULL, 5);
  dzl_preferences_add_radio (preferences, "editor", "draw-spaces", "org.gnome.builder.editor", "draw-spaces", NULL, "\"leading\"", _("Leading Only"), NULL, NULL, 6);

  dzl_preferences_add_list_group (preferences, "editor", "autosave", _("Autosave"), GTK_SELECTION_NONE, 450);
  dzl_preferences_add_switch (preferences, "editor", "autosave", "org.gnome.builder.editor", "auto-save", NULL, NULL,_("Autosave Enabled"), _("Enable or disable autosave feature"), NULL, 1);
  dzl_preferences_add_spin_button (preferences, "editor", "autosave", "org.gnome.builder.editor", "auto-save-timeout", NULL, _("Autosave Frequency"), _("The number of seconds after modification before auto saving"), NULL, 60);
}

static void
ide_preferences_builtin_register_code_insight (DzlPreferences *preferences)
{
  dzl_preferences_add_page (preferences, "code-insight", _("Code Insight"), 300);

  dzl_preferences_add_list_group (preferences, "code-insight", "highlighting", _("Highlighting"), GTK_SELECTION_NONE, 0);
  dzl_preferences_add_switch (preferences, "code-insight", "highlighting", "org.gnome.builder.code-insight", "semantic-highlighting", NULL, NULL, _("Semantic Highlighting"), _("Use code insight to highlight additional information discovered in source file"), NULL, 0);

  dzl_preferences_add_list_group (preferences, "code-insight", "diagnostics", _("Diagnostics"), GTK_SELECTION_NONE, 200);
}

static void
ide_preferences_builtin_register_completion (DzlPreferences *preferences)
{
  dzl_preferences_add_page (preferences, "completion", _("Completion"), 325);

  dzl_preferences_add_list_group (preferences, "completion", "general", _("General"), GTK_SELECTION_NONE, 0);
  dzl_preferences_add_spin_button (preferences, "completion", "general", "org.gnome.builder.editor", "completion-n-rows", NULL, _("Completions Display Size"), _("Number of completions to display"), NULL, -1);
  dzl_preferences_add_switch (preferences, "completion", "general", "org.gnome.builder.editor", "interactive-completion", NULL, NULL, _("Interactive Completion"), _("Display code suggestions interactively as you type"), NULL, 0);

  dzl_preferences_add_list_group (preferences, "completion", "providers", _("Completion Providers"), GTK_SELECTION_NONE, 100);
}

static void
ide_preferences_builtin_register_snippets (DzlPreferences *preferences)
{
  dzl_preferences_add_page (preferences, "snippets", _("Snippets"), 350);

  /* TODO: Add snippet editor widget + languages */
}

static void
language_search_changed (GtkSearchEntry      *search,
                         DzlPreferencesGroup *group)
{
  g_autoptr(DzlPatternSpec) spec = NULL;
  const gchar *text;

  g_assert (GTK_IS_SEARCH_ENTRY (search));
  g_assert (DZL_IS_PREFERENCES_GROUP (group));

  text = gtk_entry_get_text (GTK_ENTRY (search));

  if (!dzl_str_empty0 (text))
    {
      g_autofree gchar *folded = g_utf8_casefold (text, -1);

      spec = dzl_pattern_spec_new (folded);
    }

  /* FIXME:
   *
   * This is a bit of a leaky abstraction, but we can
   * clean that up later. We need to get something out
   * that is coherent for 3.22.
   */

  dzl_preferences_group_refilter (group, spec);
}

static void
ide_preferences_builtin_register_languages (DzlPreferences *preferences)
{
  GtkSourceLanguageManager *manager;
  const gchar * const *language_ids;
  GtkSearchEntry *search;
  GtkWidget *group = NULL;
  GtkWidget *flow = NULL;

  dzl_preferences_add_page (preferences, "languages", _("Programming Languages"), 200);

  manager = gtk_source_language_manager_get_default ();
  language_ids = gtk_source_language_manager_get_language_ids (manager);

  g_assert (language_ids != NULL && language_ids[0] != NULL);

  dzl_preferences_add_group (preferences, "languages", "search", NULL, 0);

  search = g_object_new (GTK_TYPE_SEARCH_ENTRY,
                         /* translators: placeholder string for the entry used to filter the languages in Preferences/Programming languages */
                         "placeholder-text", _("Search languages…"),
                         "visible", TRUE,
                         NULL);
  dzl_preferences_add_custom (preferences, "languages", "search", GTK_WIDGET (search), NULL, 0);

  dzl_preferences_add_list_group (preferences, "languages", "languages", NULL, GTK_SELECTION_SINGLE, 1);

  for (guint i = 0; language_ids [i]; i++)
    {
      g_autofree gchar *keywords = NULL;
      g_autofree gchar *folded = NULL;
      IdePreferencesLanguageRow *row;
      GtkSourceLanguage *language;
      const gchar *name;
      const gchar *section;

      language = gtk_source_language_manager_get_language (manager, language_ids [i]);
      if (gtk_source_language_get_hidden (language))
        continue;

      name = gtk_source_language_get_name (language);
      section = gtk_source_language_get_section (language);

      keywords = g_strdup_printf ("%s %s %s", name, section, language_ids [i]);
      folded = g_utf8_casefold (keywords, -1);

      row = g_object_new (IDE_TYPE_PREFERENCES_LANGUAGE_ROW,
                          "id", language_ids [i],
                          "keywords", folded,
                          "title", name,
                          "visible", TRUE,
                          NULL);
      dzl_preferences_add_custom (preferences, "languages", "languages", GTK_WIDGET (row), NULL, i);

      if G_UNLIKELY (group == NULL)
        group = gtk_widget_get_ancestor (GTK_WIDGET (row), DZL_TYPE_PREFERENCES_GROUP);
    }

  g_assert (group != NULL);

  g_signal_connect_object (search,
                           "changed",
                           G_CALLBACK (language_search_changed),
                           group,
                           0);

  g_signal_connect (search,
                    "stop-search",
                    G_CALLBACK (gtk_entry_set_text),
                    (gpointer) "");

  flow = gtk_widget_get_ancestor (group, DZL_TYPE_COLUMN_LAYOUT);

  g_assert (flow != NULL);

  dzl_column_layout_set_max_columns (DZL_COLUMN_LAYOUT (flow), 1);

  dzl_preferences_add_page (preferences, "languages.id", NULL, 0);

  dzl_preferences_add_list_group (preferences, "languages.id", "basic", _("General"), GTK_SELECTION_NONE, 0);
  dzl_preferences_add_switch (preferences, "languages.id", "basic", "org.gnome.builder.editor.language", "trim-trailing-whitespace", "/org/gnome/builder/editor/language/{id}/", NULL, _("Trim trailing whitespace"), _("Upon saving, trailing whitespace from modified lines will be trimmed."), NULL, 10);
  dzl_preferences_add_switch (preferences, "languages.id", "basic", "org.gnome.builder.editor.language", "overwrite-braces", "/org/gnome/builder/editor/language/{id}/", NULL, _("Overwrite Braces"), _("Overwrite closing braces"), NULL, 20);
  dzl_preferences_add_switch (preferences, "languages.id", "basic", "org.gnome.builder.editor.language", "insert-matching-brace", "/org/gnome/builder/editor/language/{id}/", NULL, _("Insert Matching Brace"), _("Insert matching character for { [ ( or \""), NULL, 20);
  dzl_preferences_add_switch (preferences, "languages.id", "basic", "org.gnome.builder.editor.language", "insert-trailing-newline", "/org/gnome/builder/editor/language/{id}/", NULL, _("Insert Trailing Newline"), _("Ensure files end with a newline"), NULL, 30);

  dzl_preferences_add_list_group (preferences, "languages.id", "margin", _("Margins"), GTK_SELECTION_NONE, 0);
  dzl_preferences_add_radio (preferences, "languages.id", "margin", "org.gnome.builder.editor.language", "show-right-margin", "/org/gnome/builder/editor/language/{id}/", NULL, _("Show right margin"), NULL, NULL, 0);
  dzl_preferences_add_spin_button (preferences, "languages.id", "margin", "org.gnome.builder.editor.language", "right-margin-position", "/org/gnome/builder/editor/language/{id}/", _("Right margin position"), _("Position in spaces for the right margin"), NULL, 10);

  dzl_preferences_add_list_group (preferences, "languages.id", "indentation", _("Indentation"), GTK_SELECTION_NONE, 100);
  dzl_preferences_add_spin_button (preferences, "languages.id", "indentation", "org.gnome.builder.editor.language", "tab-width", "/org/gnome/builder/editor/language/{id}/", _("Tab width"), _("Width of a tab character in spaces"), NULL, 10);
  dzl_preferences_add_radio (preferences, "languages.id", "indentation", "org.gnome.builder.editor.language", "insert-spaces-instead-of-tabs", "/org/gnome/builder/editor/language/{id}/", NULL, _("Insert spaces instead of tabs"), _("Prefer spaces over use of tabs"), NULL, 20);
  dzl_preferences_add_radio (preferences, "languages.id", "indentation", "org.gnome.builder.editor.language", "auto-indent", "/org/gnome/builder/editor/language/{id}/", NULL, _("Automatically indent"), _("Indent source code as you type"), NULL, 30);

  dzl_preferences_add_list_group (preferences, "languages.id", "spaces-style", _("Spacing"), GTK_SELECTION_NONE, 600);
  dzl_preferences_add_radio (preferences, "languages.id", "spaces-style", "org.gnome.builder.editor.language", "spaces-style", "/org/gnome/builder/editor/language/{id}/", "\"before-left-paren\"", _("Space before opening parentheses"), NULL, NULL, 0);
  dzl_preferences_add_radio (preferences, "languages.id", "spaces-style", "org.gnome.builder.editor.language", "spaces-style", "/org/gnome/builder/editor/language/{id}/", "\"before-left-bracket\"", _("Space before opening brackets"), NULL, NULL, 1);
  dzl_preferences_add_radio (preferences, "languages.id", "spaces-style", "org.gnome.builder.editor.language", "spaces-style", "/org/gnome/builder/editor/language/{id}/", "\"before-left-brace\"", _("Space before opening braces"), NULL, NULL, 2);
  dzl_preferences_add_radio (preferences, "languages.id", "spaces-style", "org.gnome.builder.editor.language", "spaces-style", "/org/gnome/builder/editor/language/{id}/", "\"before-left-angle\"", _("Space before opening angles"), NULL, NULL, 3);
  dzl_preferences_add_radio (preferences, "languages.id", "spaces-style", "org.gnome.builder.editor.language", "spaces-style", "/org/gnome/builder/editor/language/{id}/", "\"colon\"", _("Prefer a space before colons"), NULL, NULL, 4);
  dzl_preferences_add_radio (preferences, "languages.id", "spaces-style", "org.gnome.builder.editor.language", "spaces-style", "/org/gnome/builder/editor/language/{id}/", "\"comma\"", _("Prefer a space before commas"), NULL, NULL, 5);
  dzl_preferences_add_radio (preferences, "languages.id", "spaces-style", "org.gnome.builder.editor.language", "spaces-style", "/org/gnome/builder/editor/language/{id}/", "\"semicolon\"", _("Prefer a space before semicolons"), NULL, NULL, 6);
}

static gboolean
workers_output (GtkSpinButton *button)
{
  GtkAdjustment *adj = gtk_spin_button_get_adjustment (button);

  if (gtk_adjustment_get_value (adj) == -1)
    {
      gtk_entry_set_text (GTK_ENTRY (button), _("Default"));
      return TRUE;
    }
  else if (gtk_adjustment_get_value (adj) == 0)
    {
      gtk_entry_set_text (GTK_ENTRY (button), _("Number of CPU"));
      return TRUE;
    }

  return FALSE;
}

static gint
workers_input (GtkSpinButton *button,
               gdouble       *new_value)
{
  const gchar *text = gtk_entry_get_text (GTK_ENTRY (button));

  if (g_strcmp0 (text, _("Default")) == 0)
    {
      *new_value = -1;
      return TRUE;
    }
  else if (g_strcmp0 (text, _("Number of CPU")) == 0)
    {
      *new_value = 0;
      return TRUE;
    }

  return FALSE;
}

static void
ide_preferences_builtin_register_build (DzlPreferences *preferences)
{
  GtkWidget *widget, *bin;
  guint id;

  dzl_preferences_add_page (preferences, "build", _("Build"), 500);

  dzl_preferences_add_list_group (preferences, "build", "basic", _("General"), GTK_SELECTION_NONE, 0);
  id = dzl_preferences_add_spin_button (preferences, "build", "basic", "org.gnome.builder.build", "parallel", "/org/gnome/builder/build/", _("Build Workers"), _("Number of parallel build workers"), NULL, 0);

  bin = dzl_preferences_get_widget (preferences, id);
  widget = dzl_preferences_spin_button_get_spin_button (DZL_PREFERENCES_SPIN_BUTTON (bin));
  gtk_entry_set_width_chars (GTK_ENTRY (widget), 20);
  g_signal_connect (widget, "input", G_CALLBACK (workers_input), NULL);
  g_signal_connect (widget, "output", G_CALLBACK (workers_output), NULL);

  dzl_preferences_add_switch (preferences, "build", "basic", "org.gnome.builder.build", "clear-build-log-pane", NULL, NULL, _("Clear build logs"), _("Clear build log pane on rebuild"), NULL, 10);
  dzl_preferences_add_switch (preferences, "build", "basic", "org.gnome.builder", "clear-cache-at-startup", NULL, NULL, _("Clear build cache at startup"), _("Expired caches will be purged when Builder is started"), NULL, 10);

  dzl_preferences_add_list_group (preferences, "build", "network", _("Network"), GTK_SELECTION_NONE, 100);
  dzl_preferences_add_switch (preferences, "build", "network", "org.gnome.builder.build", "allow-network-when-metered", NULL, NULL, _("Allow downloads over metered connections"), _("Allow the use of metered network connections when automatically downloading dependencies"), NULL, 10);

  dzl_preferences_add_page (preferences, "debugger", _("Debugger"), 550);
  dzl_preferences_add_list_group (preferences, "debugger", "general", _("Breakpoints"), GTK_SELECTION_NONE, 0);
  dzl_preferences_add_switch (preferences, "debugger", "general", "org.gnome.builder.build", "debugger-breakpoint-on-main", NULL, NULL, _("Insert Breakpoint at Start of Application"), _("Automatically stop execution at the start of the applications main function"), NULL, 0);
}

static void
ide_preferences_builtin_register_projects (DzlPreferences *preferences)
{
  dzl_preferences_add_page (preferences, "projects", _("Projects"), 450);

  dzl_preferences_add_list_group (preferences, "projects", "directory", _("Workspace"), GTK_SELECTION_NONE, 0);
  dzl_preferences_add_file_chooser (preferences, "projects", "directory", "org.gnome.builder", "projects-directory", NULL, _("Projects directory"), _("A place for all your projects"), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, NULL, 0);
  dzl_preferences_add_switch (preferences, "projects", "directory", "org.gnome.builder", "restore-previous-files", NULL, NULL, _("Restore previously opened files"), _("Open previously opened files when loading a project"), NULL, 10);
}

#if 0
static void
author_changed_cb (DzlPreferencesEntry *entry,
                   const gchar         *text,
                   IdeVcsConfig        *conf)
{
  GValue value = G_VALUE_INIT;

  g_assert (DZL_IS_PREFERENCES_ENTRY (entry));
  g_assert (text != NULL);
  g_assert (IDE_IS_VCS_CONFIG (conf));

  g_value_init (&value, G_TYPE_STRING);
  g_value_set_string (&value, text);

  ide_vcs_config_set_config (conf, IDE_VCS_CONFIG_FULL_NAME, &value);

  g_value_unset (&value);
}

static void
email_changed_cb (DzlPreferencesEntry *entry,
                  const gchar         *text,
                  IdeVcsConfig        *conf)
{
  GValue value = G_VALUE_INIT;

  g_assert (DZL_IS_PREFERENCES_ENTRY (entry));
  g_assert (text != NULL);
  g_assert (IDE_IS_VCS_CONFIG (conf));

  g_value_init (&value, G_TYPE_STRING);
  g_value_set_string (&value, text);

  ide_vcs_config_set_config (conf, IDE_VCS_CONFIG_EMAIL, &value);

  g_value_unset (&value);
}

static void
vcs_configs_foreach_cb (PeasExtensionSet *set,
                        PeasPluginInfo   *plugin_info,
                        PeasExtension    *exten,
                        gpointer          user_data)
{
  DzlPreferences *preferences = user_data;
  IdeVcsConfig *conf = (IdeVcsConfig *)exten;
  GValue value = G_VALUE_INIT;
  GtkWidget *fullname;
  GtkWidget *email;
  GtkSizeGroup *size_group;
  g_autofree gchar *key = NULL;
  g_autofree gchar *author_name = NULL;
  g_autofree gchar *author_email = NULL;
  const gchar *name;
  const gchar *id;

  g_assert (PEAS_IS_EXTENSION_SET (set));
  g_assert (DZL_IS_PREFERENCES (preferences));
  g_assert (IDE_IS_VCS_CONFIG (conf));

  name = peas_plugin_info_get_name (plugin_info);
  id = peas_plugin_info_get_module_name (plugin_info);
  key = g_strdup_printf ("%s-config", id);

  g_object_set_data_full (G_OBJECT (preferences), key, g_object_ref (conf), g_object_unref);

  g_value_init (&value, G_TYPE_STRING);

  ide_vcs_config_get_config (conf, IDE_VCS_CONFIG_FULL_NAME, &value);
  author_name = g_strdup (g_value_get_string (&value));

  g_value_reset (&value);

  ide_vcs_config_get_config (conf, IDE_VCS_CONFIG_EMAIL, &value);
  author_email = g_strdup (g_value_get_string (&value));

  g_value_unset (&value);

  fullname = g_object_new (DZL_TYPE_PREFERENCES_ENTRY,
                           "text", dzl_str_empty0 (author_name) ? "" : author_name,
                           "title", "Author",
                           "visible", TRUE,
                           NULL);

  g_signal_connect_object (fullname,
                           "changed",
                           G_CALLBACK (author_changed_cb),
                           conf,
                           0);

  email = g_object_new (DZL_TYPE_PREFERENCES_ENTRY,
                        "text", dzl_str_empty0 (author_email) ? "" : author_email,
                        "title", "Email",
                        "visible", TRUE,
                        NULL);

  g_signal_connect_object (email,
                           "changed",
                           G_CALLBACK (email_changed_cb),
                           conf,
                           0);

  dzl_preferences_add_list_group (preferences, "vcs", id, name, GTK_SELECTION_NONE, 0);
  dzl_preferences_add_custom (preferences, "vcs", id, fullname, NULL, 0);
  dzl_preferences_add_custom (preferences, "vcs", id, email, NULL, 0);

  size_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
  gtk_size_group_add_widget (size_group, dzl_preferences_entry_get_title_widget (DZL_PREFERENCES_ENTRY (fullname)));
  gtk_size_group_add_widget (size_group, dzl_preferences_entry_get_title_widget (DZL_PREFERENCES_ENTRY (email)));
  g_clear_object (&size_group);
}

static void
ide_preferences_builtin_register_vcs (DzlPreferences *preferences)
{
  PeasEngine *engine;
  PeasExtensionSet *extensions;

  dzl_preferences_add_page (preferences, "vcs", _("Version Control"), 600);

  engine = peas_engine_get_default ();
  extensions = peas_extension_set_new (engine, IDE_TYPE_VCS_CONFIG, NULL);
  peas_extension_set_foreach (extensions, vcs_configs_foreach_cb, preferences);
  g_clear_object (&extensions);
}
#endif

static void
ide_preferences_builtin_register_sdks (DzlPreferences *preferences)
{
  /* only the page goes here, plugins will fill in the details */
  dzl_preferences_add_page (preferences, "sdk", _("SDKs"), 550);
}
#endif

static const IdePreferencePageEntry pages[] = {
  { NULL, "visual", "appearance", "org.gnome.Builder-appearance-symbolic", 0, "Appearance" },
  { NULL, "visual", "editing", "org.gnome.Builder-editing-symbolic", 10, "Editing" },
  { NULL, "visual", "keyboard", "org.gnome.Builder-shortcuts-symbolic", 20, "Shortcuts" },
  { NULL, "code", "languages", "org.gnome.Builder-languages-symbolic", 100, "Languages" },
  { NULL, "code", "completion", "org.gnome.Builder-completion-symbolic", 110, "Completion" },
  { NULL, "code", "insight", "org.gnome.Builder-diagnostics-symbolic", 120, "Diagnostics" },
  { NULL, "projects", "projects", "org.gnome.Builder-projects-symbolic", 200, "Projects" },
  { NULL, "tools", "build", "org.gnome.Builder-build-symbolic", 300, "Build" },
  { NULL, "tools", "debug", "org.gnome.Builder-debugger-symbolic", 310, "Debugger" },
  { NULL, "tools", "commands", "org.gnome.Builder-command-symbolic", 320, "Commands" },
  { NULL, "tools", "sdks", "org.gnome.Builder-sdk-symbolic", 500, "SDKs" },
  { NULL, "plugins", "plugins", "org.gnome.Builder-plugins-symbolic", 600, "Plugins" },
};

static const IdePreferenceGroupEntry groups[] = {
  { "appearance", "style", 0, "Appearance" },
  { "appearance", "preview", 0, "Style" },
  { "appearance", "schemes", 10, NULL },
  { "appearance", "font", 20, NULL },
  { "appearance", "accessories", 20, NULL },

  { "languages/*", "general", 0, "General" },
  { "languages/*", "margins", 10, "Margins" },
  { "languages/*", "spacing", 20, "Spacing" },
  { "languages/*", "indentation", 30, "Indentation" },
};

static const IdePreferenceItemEntry items[] = {
  { "appearance", "accessories", "grid", 0, ide_preferences_window_toggle, "Show Grid Pattern", "Display a grid pattern underneath source code", "org.gnome.builder.editor", NULL, "show-grid-lines" },
};

static const IdePreferenceItemEntry lang_items[] = {
  { "languages/*", "general", "trim", 0, ide_preferences_window_toggle, "Trim Trailing Whitespace", "Upon saving, trailing whitepsace from modified lines will be trimmed.", "org.gnome.builder.editor.language", "/*", "trim-trailing-whitespace" },
  { "languages/*", "general", "overwrite", 0, ide_preferences_window_toggle, "Overwrite Braces", "Overwrite closing braces", "org.gnome.builder.editor.language", "/*", "overwrite-braces" },
  { "languages/*", "general", "insert-matching", 0, ide_preferences_window_toggle, "Insert Matching Brace", "Insert matching character for [[(\"'", "org.gnome.builder.editor.language", "/*", "insert-matching-brace" },
  { "languages/*", "general", "insert-trailing", 0, ide_preferences_window_toggle, "Insert Trailing Newline", "Ensure files end with a newline", "org.gnome.builder.editor.language", "/*", "insert-trailing-newline" },

  { "languages/*", "margins", "show-right-margin", 0, ide_preferences_window_toggle, "Show right margin", "Display a margin in the editor to indicate maximum desired width", "org.gnome.builder.editor.language", "/*", "show-right-margin" },

#if 0
  { "languages/*", "spacing", "before-parens", 0, ide_preferences_window_toggle, "Prefer a space before opening parentheses" },
  { "languages/*", "spacing", "before-brackets", 0, ide_preferences_window_toggle, "Prefer a space before opening brackets" },
  { "languages/*", "spacing", "before-braces", 0, ide_preferences_window_toggle, "Prefer a space before opening braces" },
  { "languages/*", "spacing", "before-angles", 0, ide_preferences_window_toggle, "Prefer a space before opening angles" },
#endif

  { "languages/*", "indentation", "insert-spaces", 0, ide_preferences_window_toggle, "Insert spaces instead of tabs", "Prefer spaces over tabs", "org.gnome.builder.editor.language", "/*", "insert-spaces-instead-of-tabs" },
  { "languages/*", "indentation", "auto-indent", 0, ide_preferences_window_toggle, "Automatically Indent", "Format source code as you type", "org.gnome.builder.editor.language", "/*", "auto-indent" },
};

static int
compare_section (gconstpointer a,
                 gconstpointer b)
{
  const IdePreferencePageEntry *pagea = a;
  const IdePreferencePageEntry *pageb = b;

  return g_strcmp0 (pagea->section, pageb->section);
}

static void
ide_preferences_builtin_add_languages (IdePreferencesWindow *window)
{
  GtkSourceLanguageManager *langs;
  const char * const *lang_ids;
  IdePreferencePageEntry *lpages;
  guint j = 0;

  g_assert (IDE_IS_PREFERENCES_WINDOW (window));

  langs = gtk_source_language_manager_get_default ();
  lang_ids = gtk_source_language_manager_get_language_ids (langs);
  lpages = g_new0 (IdePreferencePageEntry, g_strv_length ((char **)lang_ids));

  for (guint i = 0; lang_ids[i]; i++)
    {
      GtkSourceLanguage *l = gtk_source_language_manager_get_language (langs, lang_ids[i]);
      IdePreferencePageEntry *page;
      char name[256];

      if (gtk_source_language_get_hidden (l))
        continue;

      page = &lpages[j++];

      g_snprintf (name, sizeof name, "languages/%s", lang_ids[i]);

      page->parent = "languages";
      page->section = gtk_source_language_get_section (l);
      page->name = g_intern_string (name);
      page->icon_name = NULL;
      page->title = gtk_source_language_get_name (l);
    }

  qsort (lpages, j, sizeof *lpages, compare_section);
  for (guint i = 0; i < j; i++)
    lpages[i].priority = i;

  ide_preferences_window_add_pages (window, lpages, j, NULL);
  ide_preferences_window_add_items (window, lang_items, G_N_ELEMENTS (lang_items), window, NULL);

  g_free (lpages);
}

static void
ide_preferences_builtin_add_schemes (const char                   *page_name,
                                     const IdePreferenceItemEntry *entry,
                                     AdwPreferencesGroup          *group,
                                     gpointer                      user_data)
{
  IdePreferencesWindow *window = user_data;
  g_autoptr(GtkSourceBuffer) buffer = NULL;
  GtkSourceStyleSchemeManager *manager;
  const char * const *scheme_ids;
  GtkSourceLanguage *language;
  GtkSourceView *preview;
  GtkFlowBox *flowbox;
  GtkFrame *frame;

  g_assert (IDE_IS_PREFERENCES_WINDOW (window));
  g_assert (entry != NULL);
  g_assert (ADW_IS_PREFERENCES_GROUP (group));

#define PREVIEW_TEXT "\
#include <glib.h>\n\
"

  language = gtk_source_language_manager_get_language (gtk_source_language_manager_get_default (), "c");
  buffer = g_object_new (GTK_SOURCE_TYPE_BUFFER,
                         "language", language,
                         "text", PREVIEW_TEXT,
                         NULL);
  preview = g_object_new (GTK_SOURCE_TYPE_VIEW,
                          "buffer", buffer,
                          "monospace", TRUE,
                          "left-margin", 6,
                          "top-margin", 6,
                          "bottom-margin", 6,
                          "right-margin", 6,
                          "show-line-numbers", TRUE,
                          "highlight-current-line", TRUE,
                          "show-right-margin", TRUE,
                          "right-margin-position", 60,
                          NULL);
  frame = g_object_new (GTK_TYPE_FRAME,
                        "child", preview,
                        "margin-bottom", 12,
                        NULL);
  adw_preferences_group_add (group, GTK_WIDGET (frame));

  manager = gtk_source_style_scheme_manager_get_default ();
  scheme_ids = gtk_source_style_scheme_manager_get_scheme_ids (manager);

  flowbox = g_object_new (GTK_TYPE_FLOW_BOX,
                          "activate-on-single-click", TRUE,
                          "column-spacing", 6,
                          "row-spacing", 6,
                          "max-children-per-line", 4,
                          NULL);

  for (guint i = 0; scheme_ids[i]; i++)
    {
      GtkSourceStyleScheme *scheme = gtk_source_style_scheme_manager_get_scheme (manager, scheme_ids[i]);
      GtkWidget *selector = gtk_source_style_scheme_preview_new (scheme);

      gtk_flow_box_append (flowbox, selector);
    }

  adw_preferences_group_add (group, GTK_WIDGET (flowbox));
}

void
_ide_preferences_builtin_register (IdePreferencesWindow *window)
{
  IdePreferencesMode mode;

  g_return_if_fail (IDE_IS_PREFERENCES_WINDOW (window));

  mode = ide_preferences_window_get_mode (window);

  if (mode == IDE_PREFERENCES_MODE_APPLICATION)
    {
      ide_preferences_window_add_pages (window, pages, G_N_ELEMENTS (pages), NULL);
      ide_preferences_window_add_groups (window, groups, G_N_ELEMENTS (groups), NULL);
      ide_preferences_window_add_items (window, items, G_N_ELEMENTS (items), window, NULL);
      ide_preferences_window_add_item (window, "appearance", "preview", "scheme", 0,
                                       ide_preferences_builtin_add_schemes, window, NULL);

      ide_preferences_builtin_add_languages (window);
    }
  else if (mode == IDE_PREFERENCES_MODE_PROJECT)
    {
    }
}
