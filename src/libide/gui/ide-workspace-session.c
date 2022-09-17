/* ide-workspace-session.c
 *
 * Copyright 2022 Christian Hergert <chergert@redhat.com>
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

#define G_LOG_DOMAIN "ide-workspace-session"

#include "config.h"

#include <libpeas/peas.h>

#include <libide-plugins.h>

#include "ide-frame.h"
#include "ide-grid.h"
#include "ide-gui-global.h"
#include "ide-session.h"
#include "ide-session-item.h"
#include "ide-workspace-addin.h"
#include "ide-workspace-private.h"

static void
ide_workspace_addin_save_session_cb (IdeExtensionSetAdapter *adapter,
                                     PeasPluginInfo         *plugin_info,
                                     PeasExtension          *exten,
                                     gpointer                user_data)
{
  IdeWorkspaceAddin *addin = (IdeWorkspaceAddin *)exten;
  IdeSession *session = user_data;

  g_assert (IDE_IS_EXTENSION_SET_ADAPTER (adapter));
  g_assert (plugin_info != NULL);
  g_assert (IDE_IS_WORKSPACE_ADDIN (addin));
  g_assert (IDE_IS_SESSION (session));

  ide_workspace_addin_save_session (addin, session);
}

void
_ide_workspace_save_session (IdeWorkspace *self,
                             IdeSession   *session)
{
  IDE_ENTRY;

  g_return_if_fail (IDE_IS_WORKSPACE (self));
  g_return_if_fail (IDE_IS_SESSION (session));

  if (IDE_WORKSPACE_GET_CLASS (self)->save_session)
    IDE_WORKSPACE_GET_CLASS (self)->save_session (self, session);

  ide_extension_set_adapter_foreach (_ide_workspace_get_addins (self),
                                     ide_workspace_addin_save_session_cb,
                                     session);

  IDE_EXIT;
}

static void
ide_workspace_save_session_frame_cb (PanelFrame *frame,
                                     gpointer    user_data)
{
  g_autoptr(PanelPosition) position = NULL;
  g_autoptr(IdeSessionItem) item = NULL;
  IdeSession *session = user_data;
  IdeWorkspace *workspace;
  int requested_size;

  IDE_ENTRY;

  g_assert (IDE_IS_MAIN_THREAD ());
  g_assert (PANEL_IS_FRAME (frame));
  g_assert (IDE_IS_SESSION (session));

  position = panel_frame_get_position (frame);
  workspace = ide_widget_get_workspace (GTK_WIDGET (frame));
  requested_size = panel_frame_get_requested_size (frame);

#if 0
  dump_position (position);
#endif

  item = ide_session_item_new ();
  ide_session_item_set_module_name (item, "libide-gui");
  ide_session_item_set_type_hint (item, G_OBJECT_TYPE_NAME (frame));
  ide_session_item_set_position (item, position);
  ide_session_item_set_workspace (item, ide_workspace_get_id (workspace));

  if (requested_size > -1)
    ide_session_item_set_metadata (item, "size", "i", requested_size);

  ide_session_append (session, item);

  IDE_EXIT;
}

void
_ide_workspace_save_session_simple (IdeWorkspace     *self,
                                    IdeSession       *session,
                                    IdeWorkspaceDock *dock)
{
  g_autoptr(IdeSessionItem) item = NULL;
  gboolean reveal_start;
  gboolean reveal_end;
  gboolean reveal_bottom;
  int start_width;
  int end_width;
  int bottom_height;
  int width;
  int height;

  IDE_ENTRY;

  g_return_if_fail (IDE_IS_WORKSPACE (self));
  g_return_if_fail (IDE_IS_SESSION (session));

  gtk_window_get_default_size (GTK_WINDOW (self), &width, &height);

  item = ide_session_item_new ();
  ide_session_item_set_id (item, ide_workspace_get_id (self));
  ide_session_item_set_workspace (item, ide_workspace_get_id (self));
  ide_session_item_set_module_name (item, "libide-gui");
  ide_session_item_set_type_hint (item, G_OBJECT_TYPE_NAME (self));
  ide_session_item_set_metadata (item, "size", "(ii)", width, height);
  if (gtk_window_is_active (GTK_WINDOW (self)))
    ide_session_item_set_metadata (item, "is-active", "b", TRUE);
  if (gtk_window_is_maximized (GTK_WINDOW (self)))
    ide_session_item_set_metadata (item, "is-maximized", "b", TRUE);

  g_object_get (dock->dock,
                "reveal-start", &reveal_start,
                "reveal-end", &reveal_end,
                "reveal-bottom", &reveal_bottom,
                "start-width", &start_width,
                "end-width", &end_width,
                "bottom-height", &bottom_height,
                NULL);

  ide_session_item_set_metadata (item, "reveal-start", "b", reveal_start);
  ide_session_item_set_metadata (item, "reveal-end", "b", reveal_end);
  ide_session_item_set_metadata (item, "reveal-bottom", "b", reveal_bottom);

  ide_session_item_set_metadata (item, "start-width", "i", start_width);
  ide_session_item_set_metadata (item, "end-width", "i", end_width);
  ide_session_item_set_metadata (item, "bottom-height", "i", bottom_height);

#if 0
  g_print ("Saving %d %d %d %d %d %d\n",
           reveal_start, reveal_end, reveal_bottom,
           start_width, end_width, bottom_height);
#endif

  ide_session_prepend (session, item);

  panel_dock_foreach_frame (dock->dock,
                            ide_workspace_save_session_frame_cb,
                            session);
  panel_grid_foreach_frame (PANEL_GRID (dock->grid),
                            ide_workspace_save_session_frame_cb,
                            session);

  IDE_EXIT;
}

static void
ide_workspace_addin_restore_session_cb (IdeExtensionSetAdapter *adapter,
                                        PeasPluginInfo         *plugin_info,
                                        PeasExtension          *exten,
                                        gpointer                user_data)
{
  IdeWorkspaceAddin *addin = (IdeWorkspaceAddin *)exten;
  IdeSession *session = user_data;

  g_assert (IDE_IS_EXTENSION_SET_ADAPTER (adapter));
  g_assert (plugin_info != NULL);
  g_assert (IDE_IS_WORKSPACE_ADDIN (addin));
  g_assert (IDE_IS_SESSION (session));

  ide_workspace_addin_restore_session (addin, session);
}

void
_ide_workspace_restore_session (IdeWorkspace *self,
                                IdeSession   *session)
{
  IDE_ENTRY;

  g_return_if_fail (IDE_IS_WORKSPACE (self));
  g_return_if_fail (IDE_IS_SESSION (session));

  if (IDE_WORKSPACE_GET_CLASS (self)->restore_session)
    IDE_WORKSPACE_GET_CLASS (self)->restore_session (self, session);

  ide_extension_set_adapter_foreach (_ide_workspace_get_addins (self),
                                     ide_workspace_addin_restore_session_cb,
                                     session);

  IDE_EXIT;
}

static void
ide_workspace_restore_frame (IdeWorkspace     *self,
                             GType             type,
                             IdeSessionItem   *item,
                             IdeWorkspaceDock *dock)
{
  PanelPosition *position;
  GtkWidget *frame;
  PanelArea area;

  g_assert (IDE_IS_WORKSPACE (self));
  g_assert (g_type_is_a (type, PANEL_TYPE_FRAME));
  g_assert (IDE_IS_SESSION_ITEM (item));

  if (!(position = ide_session_item_get_position (item)))
    return;

  if (!panel_position_get_area_set (position))
    return;

  area = panel_position_get_area (position);
  if ((area == PANEL_AREA_CENTER && type != IDE_TYPE_FRAME) ||
      (area != PANEL_AREA_CENTER && type != PANEL_TYPE_FRAME))
    return;

  if (area == PANEL_AREA_START || area == PANEL_AREA_END)
    {
      PanelPaned *paned = area == PANEL_AREA_START ? dock->start_area : dock->end_area;
      int row = panel_position_get_row (position);

      while (panel_paned_get_n_children (paned) <= row)
        {
          frame = panel_frame_new ();
          panel_paned_append (paned, GTK_WIDGET (frame));
        }

      frame = panel_paned_get_nth_child (paned, row);
    }
  else if (area == PANEL_AREA_TOP)
    {
      /* Ignored */
      return;
    }
  else if (area == PANEL_AREA_BOTTOM)
    {
      PanelPaned *paned = dock->bottom_area;
      int column = panel_position_get_column (position);

      while (panel_paned_get_n_children (paned) <= column)
        {
          frame = panel_frame_new ();
          panel_paned_append (paned, GTK_WIDGET (frame));
        }

      frame = panel_paned_get_nth_child (paned, column);
    }
  else
    {
      int column = panel_position_get_column (position);
      int row = panel_position_get_row (position);

      frame = GTK_WIDGET (ide_grid_make_frame (dock->grid, column, row));
    }

  if (ide_session_item_has_metadata_with_type (item, "size", G_VARIANT_TYPE ("i")))
    {
      int size;

      ide_session_item_get_metadata (item, "size", "i", &size);
      panel_frame_set_requested_size (PANEL_FRAME (frame), size);
    }
}

static void
ide_workspace_restore_panels (IdeWorkspace     *self,
                              IdeSessionItem   *item,
                              IdeWorkspaceDock *dock)
{
  gboolean reveal_start = -1;
  gboolean reveal_end = -1;
  gboolean reveal_bottom = -1;
  int start_width = -1;
  int end_width = -1;
  int bottom_height = -1;

  g_return_if_fail (IDE_IS_WORKSPACE (self));
  g_return_if_fail (IDE_IS_SESSION_ITEM (item));
  g_return_if_fail (dock != NULL);

  ide_session_item_get_metadata (item, "reveal-start", "b", &reveal_start);
  ide_session_item_get_metadata (item, "reveal-end", "b", &reveal_end);
  ide_session_item_get_metadata (item, "reveal-bottom", "b", &reveal_bottom);
  ide_session_item_get_metadata (item, "start-width", "i", &start_width);
  ide_session_item_get_metadata (item, "end-width", "i", &end_width);
  ide_session_item_get_metadata (item, "bottom-height", "i", &bottom_height);

#if 0
  g_print ("Restoring %d %d %d %d %d %d\n",
           reveal_start, reveal_end, reveal_bottom,
           start_width, end_width, bottom_height);
#endif

  if (reveal_start > -1)
    panel_dock_set_reveal_start (dock->dock, reveal_start);

  if (reveal_end > -1)
    panel_dock_set_reveal_end (dock->dock, reveal_end);

  if (reveal_bottom > -1)
    panel_dock_set_reveal_bottom (dock->dock, reveal_bottom);

  if (start_width > -1)
    panel_dock_set_start_width (dock->dock, start_width);

  if (end_width > -1)
    panel_dock_set_end_width (dock->dock, end_width);

  if (bottom_height > -1)
    panel_dock_set_bottom_height (dock->dock, bottom_height);
}

void
_ide_workspace_restore_session_simple (IdeWorkspace     *self,
                                       IdeSession       *session,
                                       IdeWorkspaceDock *dock)
{
  guint n_items;

  IDE_ENTRY;

  g_return_if_fail (IDE_IS_WORKSPACE (self));
  g_return_if_fail (IDE_IS_SESSION (session));
  g_return_if_fail (dock != NULL);

  n_items = ide_session_get_n_items (session);

  for (guint i = 0; i < n_items; i++)
    {
      IdeSessionItem *item = ide_session_get_item (session, i);
      const char *module_name = ide_session_item_get_module_name (item);

      if (ide_str_equal0 (module_name, "libide-gui"))
        {
          const char *workspace_id = ide_session_item_get_workspace (item);
          const char *type_hint = ide_session_item_get_type_hint (item);
          GType type = type_hint ? g_type_from_name (type_hint) : G_TYPE_INVALID;

          if (type == G_TYPE_INVALID)
            continue;

          if (!ide_str_equal0 (workspace_id, ide_workspace_get_id (self)))
            continue;

          if (g_type_is_a (type, PANEL_TYPE_FRAME))
            ide_workspace_restore_frame (self, type, item, dock);
          else if (g_type_is_a (type, IDE_TYPE_WORKSPACE) &&
                   type == G_OBJECT_TYPE (self))
            ide_workspace_restore_panels (self, item, dock);
        }
    }

  IDE_EXIT;
}
